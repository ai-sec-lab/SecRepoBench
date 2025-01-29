if(extension_id == 11 /* ec_point_formats groups */) {
		u_int16_t s_offset = offset+extension_offset + 1;

#ifdef DEBUG_TLS
		printf("Client TLS [EllipticCurveFormat: len=%u]\n", extension_len);
#endif
		if((s_offset+extension_len-1) <= total_len) {
		  for(i=0; i<(u_int32_t)extension_len-1 && s_offset+i < total_len; i++) {
		    u_int8_t s_group = packet->payload[s_offset+i];

#ifdef DEBUG_TLS
		    printf("Client TLS [EllipticCurveFormat: %u]\n", s_group);
#endif

		    if(ja3.client.num_elliptic_curve_point_format < MAX_NUM_JA3)
		      ja3.client.elliptic_curve_point_format[ja3.client.num_elliptic_curve_point_format++] = s_group;
		    else {
		      invalid_ja3 = 1;
#ifdef DEBUG_TLS
		      printf("Client TLS Invalid num elliptic %u\n", ja3.client.num_elliptic_curve_point_format);
#endif
		    }
		  }
		} else {
		  invalid_ja3 = 1;
#ifdef DEBUG_TLS
		  printf("Client TLS Invalid len %u vs %u\n", s_offset+extension_len, total_len);
#endif
		}
	      } else if(extension_id == 13 /* signature algorithms */) {
		int s_offset = offset+extension_offset, safari_signature_algorithms = 0, chrome_signature_algorithms = 0,
		  duplicate_found = 0, last_signature = 0;
		u_int16_t tot_signature_algorithms_len = ntohs(*((u_int16_t*)&packet->payload[s_offset]));

#ifdef DEBUG_TLS
		printf("Client TLS [SIGNATURE_ALGORITHMS: block_len=%u/len=%u]\n", extension_len, tot_signature_algorithms_len);
#endif

		s_offset += 2;
		tot_signature_algorithms_len = ndpi_min((sizeof(ja3.client.signature_algorithms) / 2) - 1, tot_signature_algorithms_len);

#ifdef TLS_HANDLE_SIGNATURE_ALGORITMS
		flow->protos.tls_quic.num_tls_signature_algorithms = ndpi_min(tot_signature_algorithms_len / 2, MAX_NUM_TLS_SIGNATURE_ALGORITHMS);

		memcpy(flow->protos.tls_quic.client_signature_algorithms,
		       &packet->payload[s_offset], 2 /* 16 bit */*flow->protos.tls_quic.num_tls_signature_algorithms);
#endif

		for(i=0; i<tot_signature_algorithms_len && s_offset+i<total_len; i++) {
		  int rc = snprintf(&ja3.client.signature_algorithms[i*2], sizeof(ja3.client.signature_algorithms)-i*2, "%02X", packet->payload[s_offset+i]);

		  if(rc < 0) break;
		}

		for(i=0; i<tot_signature_algorithms_len && s_offset + (int)i + 2 < packet->payload_packet_len; i+=2) {
		  u_int16_t signature_algo = (u_int16_t)ntohs(*((u_int16_t*)&packet->payload[s_offset+i]));

		  if(last_signature == signature_algo) {
		    /* Consecutive duplication */
		    duplicate_found = 1;
		    continue;
		  } else {
		    /* Check for other duplications */
		    u_int all_ok = 1;

		    for(j=0; j<tot_signature_algorithms_len; j+=2) {
		      if(j != i && s_offset + (int)j + 2 < packet->payload_packet_len) {
			u_int16_t j_signature_algo = (u_int16_t)ntohs(*((u_int16_t*)&packet->payload[s_offset+j]));

			if((signature_algo == j_signature_algo)
			   && (i < j) /* Don't skip both of them */) {
#ifdef DEBUG_HEURISTIC
			  printf("[SIGNATURE] [TLS Signature Algorithm] Skipping duplicate 0x%04X\n", signature_algo);
#endif

			  duplicate_found = 1, all_ok = 0;
			  break;
			}
		      }
		    }

		    if(!all_ok)
		      continue;
		  }

		  last_signature = signature_algo;

#ifdef DEBUG_HEURISTIC
		  printf("[SIGNATURE] [TLS Signature Algorithm] 0x%04X\n", signature_algo);
#endif
		  switch(signature_algo) {
		  case ECDSA_SECP521R1_SHA512:
		    flow->protos.tls_quic.browser_heuristics.is_firefox_tls = 1;
		    break;

		  case ECDSA_SECP256R1_SHA256:
		  case ECDSA_SECP384R1_SHA384:
		  case RSA_PKCS1_SHA256:
		  case RSA_PKCS1_SHA384:
		  case RSA_PKCS1_SHA512:
		  case RSA_PSS_RSAE_SHA256:
		  case RSA_PSS_RSAE_SHA384:
		  case RSA_PSS_RSAE_SHA512:
		    chrome_signature_algorithms++, safari_signature_algorithms++;
#ifdef DEBUG_HEURISTIC
		    printf("[SIGNATURE] [Chrome/Safari] Found 0x%04X [chrome: %u][safari: %u]\n",
			   signature_algo, chrome_signature_algorithms, safari_signature_algorithms);
#endif

		    break;
		  }
		}

#ifdef DEBUG_HEURISTIC
		printf("[SIGNATURE] [safari_signature_algorithms: %u][chrome_signature_algorithms: %u]\n",
		       safari_signature_algorithms, chrome_signature_algorithms);
#endif

		if(flow->protos.tls_quic.browser_heuristics.is_firefox_tls)
		  flow->protos.tls_quic.browser_heuristics.is_safari_tls = 0,
		    flow->protos.tls_quic.browser_heuristics.is_chrome_tls = 0;

		if(safari_signature_algorithms != 8)
		   flow->protos.tls_quic.browser_heuristics.is_safari_tls = 0;

		if((chrome_signature_algorithms != 8) || duplicate_found)
		   flow->protos.tls_quic.browser_heuristics.is_chrome_tls = 0;

		/* Avoid Chrome and Safari overlaps, thing that cannot happen with Firefox */
		if(flow->protos.tls_quic.browser_heuristics.is_safari_tls)
		  flow->protos.tls_quic.browser_heuristics.is_chrome_tls = 0;

		if((flow->protos.tls_quic.browser_heuristics.is_chrome_tls == 0)
		   && duplicate_found)
		  flow->protos.tls_quic.browser_heuristics.is_safari_tls = 1; /* Safari */

#ifdef DEBUG_HEURISTIC
		printf("[SIGNATURE] [is_firefox_tls: %u][is_chrome_tls: %u][is_safari_tls: %u][duplicate_found: %u]\n",
		       flow->protos.tls_quic.browser_heuristics.is_firefox_tls,
		       flow->protos.tls_quic.browser_heuristics.is_chrome_tls,
		       flow->protos.tls_quic.browser_heuristics.is_safari_tls,
		       duplicate_found);
#endif

		if(i > 0 && i >= tot_signature_algorithms_len) {
		  ja3.client.signature_algorithms[i*2 - 1] = '\0';
		} else {
		  ja3.client.signature_algorithms[i*2] = '\0';
		}

#ifdef DEBUG_TLS
		printf("Client TLS [SIGNATURE_ALGORITHMS: %s]\n", ja3.client.signature_algorithms);
#endif
	      } else if(extension_id == 16 /* application_layer_protocol_negotiation */ &&
	                offset+extension_offset+1 < total_len) {
		u_int16_t s_offset = offset+extension_offset;
		u_int16_t tot_alpn_len = ntohs(*((u_int16_t*)&packet->payload[s_offset]));
		char alpn_str[256];
		u_int8_t alpn_str_len = 0, i;

#ifdef DEBUG_TLS
		printf("Client TLS [ALPN: block_len=%u/len=%u]\n", extension_len, tot_alpn_len);
#endif
		s_offset += 2;
		tot_alpn_len += s_offset;

		while(s_offset < tot_alpn_len && s_offset < total_len) {
		  u_int8_t alpn_i, alpn_len = packet->payload[s_offset++];

		  if((s_offset + alpn_len) <= tot_alpn_len &&
		     (s_offset + alpn_len) <= total_len) {
#ifdef DEBUG_TLS
		    printf("Client TLS [ALPN: %u]\n", alpn_len);
#endif

		    if(((uint32_t)alpn_str_len+alpn_len+1) < (sizeof(alpn_str)-1)) {
		      if(alpn_str_len > 0) {
			alpn_str[alpn_str_len] = ',';
			alpn_str_len++;
		      }

		      for(alpn_i=0; alpn_i<alpn_len; alpn_i++)
			alpn_str[alpn_str_len+alpn_i] = packet->payload[s_offset+alpn_i];

		      s_offset += alpn_len, alpn_str_len += alpn_len;;
		    } else
		      break;
		  } else
		    break;
		} /* while */

		alpn_str[alpn_str_len] = '\0';

#ifdef DEBUG_TLS
		printf("Client TLS [ALPN: %s][len: %u]\n", alpn_str, alpn_str_len);
#endif
		if(flow->protos.tls_quic.alpn == NULL)
		  flow->protos.tls_quic.alpn = ndpi_strdup(alpn_str);

		snprintf(ja3.client.alpn, sizeof(ja3.client.alpn), "%s", alpn_str);

		/* Replace , with - as in JA3 */
		for(i=0; ja3.client.alpn[i] != '\0'; i++)
		  if(ja3.client.alpn[i] == ',') ja3.client.alpn[i] = '-';

	      } else if(extension_id == 43 /* supported versions */) {
		u_int16_t s_offset = offset+extension_offset;
		u_int8_t version_len = packet->payload[s_offset];
		char version_str[256];
		char buf_ver_tmp[16];
		size_t version_str_len = 0;
		version_str[0] = 0;
#ifdef DEBUG_TLS
		printf("Client TLS [TLS version len: %u]\n", version_len);
#endif

		if(version_len == (extension_len-1)) {
		  u_int8_t j;
		  u_int16_t supported_versions_offset = 0;

		  s_offset++;

		  // careful not to overflow and loop forever with u_int8_t
		  for(j=0; j+1<version_len && s_offset + j + 1 < packet->payload_packet_len; j += 2) {
		    u_int16_t tls_version = ntohs(*((u_int16_t*)&packet->payload[s_offset+j]));
		    u_int8_t unknown_tls_version;

#ifdef DEBUG_TLS
		    printf("Client TLS [TLS version: %s/0x%04X]\n",
			   ndpi_ssl_version2str(buf_ver_tmp, sizeof(buf_ver_tmp), tls_version, &unknown_tls_version), tls_version);
#endif

		    if((version_str_len+8) < sizeof(version_str)) {
		      int rc = snprintf(&version_str[version_str_len],
					sizeof(version_str) - version_str_len, "%s%s",
					(version_str_len > 0) ? "," : "",
					ndpi_ssl_version2str(buf_ver_tmp, sizeof(buf_ver_tmp), tls_version, &unknown_tls_version));
		      if(rc <= 0)
			break;
		      else
			version_str_len += rc;

		      rc = snprintf(&ja3.client.supported_versions[supported_versions_offset],
				    sizeof(ja3.client.supported_versions)-supported_versions_offset,
				    "%s%04X", (j > 0) ? "-" : "", tls_version);

		      if(rc > 0)
			supported_versions_offset += rc;
		    }
		  }

#ifdef DEBUG_TLS
		  printf("Client TLS [SUPPORTED_VERSIONS: %s]\n", ja3.client.supported_versions);
#endif

		  if(flow->protos.tls_quic.tls_supported_versions == NULL)
		    flow->protos.tls_quic.tls_supported_versions = ndpi_strdup(version_str);
		}
	      } else if(extension_id == 65486 /* encrypted server name */ &&
	                offset+extension_offset+1 < total_len) {
		/*
		   - https://tools.ietf.org/html/draft-ietf-tls-esni-06
		   - https://blog.cloudflare.com/encrypted-sni/
		*/
		int e_offset = offset+extension_offset;
		int e_sni_len;
		int initial_offset = e_offset;
		u_int16_t cipher_suite = ntohs(*((u_int16_t*)&packet->payload[e_offset]));

		flow->protos.tls_quic.encrypted_sni.cipher_suite = cipher_suite;

		e_offset += 2; /* Cipher suite len */

		/* Key Share Entry */
		e_offset += 2; /* Group */
		if(e_offset + 2 < packet->payload_packet_len) {
		e_offset += ntohs(*((u_int16_t*)&packet->payload[e_offset])) + 2; /* Lenght */

		if((e_offset+4) < packet->payload_packet_len) {
		  /* Record Digest */
		  e_offset +=  ntohs(*((u_int16_t*)&packet->payload[e_offset])) + 2; /* Lenght */

		  if((e_offset+4) < packet->payload_packet_len) {
		    e_sni_len = ntohs(*((u_int16_t*)&packet->payload[e_offset]));
		    e_offset += 2;

		    if((e_offset+e_sni_len-(int)extension_len-initial_offset) >= 0 &&
		        e_offset+e_sni_len < packet->payload_packet_len) {
#ifdef DEBUG_ENCRYPTED_SNI
		      printf("Client TLS [Encrypted Server Name len: %u]\n", e_sni_len);
#endif

		      if(flow->protos.tls_quic.encrypted_sni.esni == NULL) {
			flow->protos.tls_quic.encrypted_sni.esni = (char*)ndpi_malloc(e_sni_len*2+1);

			if(flow->protos.tls_quic.encrypted_sni.esni) {
			  u_int16_t off;
			  int i;

			  for(i=e_offset, off=0; i<(e_offset+e_sni_len); i++) {
			    int rc = sprintf(&flow->protos.tls_quic.encrypted_sni.esni[off], "%02X", packet->payload[i] & 0XFF);

			    if(rc <= 0) {
			      flow->protos.tls_quic.encrypted_sni.esni[off] = '\0';
			      break;
			    } else
			      off += rc;
			  }
			}
		      }
		    }
		  }
		}
		}
	      } else if(extension_id == 65445 || /* QUIC transport parameters (drafts version) */
		        extension_id == 57) { /* QUIC transport parameters (final version) */
		u_int16_t s_offset = offset+extension_offset;
		uint16_t final_offset;
		int using_var_int = is_version_with_var_int_transport_params(quic_version);

		if(!using_var_int) {
		  if(s_offset+1 >= total_len) {
		    final_offset = 0; /* Force skipping extension */
		  } else {
		    u_int16_t seq_len = ntohs(*((u_int16_t*)&packet->payload[s_offset]));
		    s_offset += 2;
	            final_offset = MIN(total_len, s_offset + seq_len);
		  }
		} else {
	          final_offset = MIN(total_len, s_offset + extension_len);
		}

		while(s_offset < final_offset) {
		  u_int64_t param_type, param_len;

                  if(!using_var_int) {
		    if(s_offset+3 >= final_offset)
		      break;
		    param_type = ntohs(*((u_int16_t*)&packet->payload[s_offset]));
		    param_len = ntohs(*((u_int16_t*)&packet->payload[s_offset + 2]));
		    s_offset += 4;
		  } else {
		    if(s_offset >= final_offset ||
		       (s_offset + quic_len_buffer_still_required(packet->payload[s_offset])) >= final_offset)
		      break;
		    s_offset += quic_len(&packet->payload[s_offset], &param_type);

		    if(s_offset >= final_offset ||
		       (s_offset + quic_len_buffer_still_required(packet->payload[s_offset])) >= final_offset)
		      break;
		    s_offset += quic_len(&packet->payload[s_offset], &param_len);
		  }

#ifdef DEBUG_TLS
		  printf("Client TLS [QUIC TP: Param 0x%x Len %d]\n", (int)param_type, (int)param_len);
#endif
		  if(s_offset+param_len > final_offset)
		    break;

		  if(param_type==0x3129) {
#ifdef DEBUG_TLS
		      printf("UA [%.*s]\n", (int)param_len, &packet->payload[s_offset]);
#endif
		      http_process_user_agent(ndpi_struct, flow,
					      &packet->payload[s_offset], param_len);
		      break;
		  }
		  s_offset += param_len;
		}
	      }