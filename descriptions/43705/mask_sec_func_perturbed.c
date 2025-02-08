int processClientServerHello(struct ndpi_detection_module_struct *ndpi_struct,
			     struct ndpi_flow_struct *flow, uint32_t quic_version) {
  struct ndpi_packet_struct *packet = &ndpi_struct->packet;
  union ja3_info ja3;
  u_int8_t invalid_ja3 = 0;
  u_int16_t tls_version, ja3_str_len;
  char ja3_str[JA3_STR_LEN];
  ndpi_MD5_CTX ctx;
  u_char md5_hash[16];
  u_int32_t i, j;
  u_int16_t total_len;
  u_int8_t handshake_type;
  int is_quic = (quic_version != 0);
  int isdatagramtls = packet->udp && (!is_quic);

#ifdef DEBUG_TLS
  printf("TLS %s() called\n", __FUNCTION__);
#endif


  handshake_type = packet->payload[0];
  total_len = (packet->payload[1] << 16) +  (packet->payload[2] << 8) + packet->payload[3];

  if((total_len > packet->payload_packet_len) || (packet->payload[1] != 0x0))
    return(0); /* Not found */

  total_len = packet->payload_packet_len;

  /* At least "magic" 3 bytes, null for string end, otherwise no need to waste cpu cycles */
  if(total_len > 4) {
    u_int16_t base_offset    = (!isdatagramtls) ? 38 : 46;
    u_int16_t version_offset = (!isdatagramtls) ? 4 : 12;
    u_int16_t offset = (!isdatagramtls) ? 38 : 46;
    u_int32_t tot_extension_len;
    u_int8_t  session_id_len =  0;

    if((base_offset >= total_len) ||
       (version_offset + 1) >= total_len)
      return 0; /* Not found */

    session_id_len = packet->payload[base_offset];

#ifdef DEBUG_TLS
    printf("TLS [len: %u][handshake_type: %02X]\n", packet->payload_packet_len, handshake_type);
#endif

    tls_version = ntohs(*((u_int16_t*)&packet->payload[version_offset]));

    if(handshake_type == 0x02 /* Server Hello */) {
      int rc;

      ja3.server.num_cipher = 0;
      ja3.server.num_tls_extension = 0;
      ja3.server.num_elliptic_curve_point_format = 0;
      ja3.server.alpn[0] = '\0';

      ja3.server.tls_handshake_version = tls_version;

#ifdef DEBUG_TLS
      printf("TLS Server Hello [version: 0x%04X]\n", tls_version);
#endif

      /*
	The server hello decides about the TLS version of this flow
	https://networkengineering.stackexchange.com/questions/55752/why-does-wireshark-show-version-tls-1-2-here-instead-of-tls-1-3
      */
      if(packet->udp)
	offset += session_id_len + 1;
      else {
	if(tls_version < 0x7F15 /* TLS 1.3 lacks of session id */)
	  offset += session_id_len+1;
      }

      if((offset+3) > packet->payload_packet_len)
	return(0); /* Not found */

      ja3.server.num_cipher = 1, ja3.server.cipher[0] = ntohs(*((u_int16_t*)&packet->payload[offset]));
      if((flow->protos.tls_quic.server_unsafe_cipher = ndpi_is_safe_ssl_cipher(ja3.server.cipher[0])) == 1)
	ndpi_set_risk(ndpi_struct, flow, NDPI_TLS_WEAK_CIPHER);

      flow->protos.tls_quic.server_cipher = ja3.server.cipher[0];

#ifdef DEBUG_TLS
      printf("TLS [server][session_id_len: %u][cipher: %04X]\n", session_id_len, ja3.server.cipher[0]);
#endif

      offset += 2 + 1;

      if((offset + 1) < packet->payload_packet_len) /* +1 because we are goint to read 2 bytes */
	tot_extension_len = ntohs(*((u_int16_t*)&packet->payload[offset]));
      else
	tot_extension_len = 0;

#ifdef DEBUG_TLS
      printf("TLS [server][tot_extension_len: %u]\n", tot_extension_len);
#endif
      offset += 2;

      for(i=0; i<tot_extension_len; ) {
        u_int16_t extension_id;
        u_int32_t extension_len;

	if((offset+4) > packet->payload_packet_len) break;

	extension_id  = ntohs(*((u_int16_t*)&packet->payload[offset]));
	extension_len = ntohs(*((u_int16_t*)&packet->payload[offset+2]));
	if(offset+4+extension_len > packet->payload_packet_len) {
	  break;
	}

	if(ja3.server.num_tls_extension < MAX_NUM_JA3)
	  ja3.server.tls_extension[ja3.server.num_tls_extension++] = extension_id;

#ifdef DEBUG_TLS
	printf("TLS [server][extension_id: %u/0x%04X][len: %u]\n",
	       extension_id, extension_id, extension_len);
#endif
	checkExtensions(ndpi_struct, flow, isdatagramtls, extension_id, extension_len, offset + 4);

	if(extension_id == 43 /* supported versions */) {
	  if(extension_len >= 2) {
	    u_int16_t tls_version = ntohs(*((u_int16_t*)&packet->payload[offset+4]));

#ifdef DEBUG_TLS
	    printf("TLS [server] [TLS version: 0x%04X]\n", tls_version);
#endif

	    flow->protos.tls_quic.ssl_version = ja3.server.tls_supported_version = tls_version;
	  }
	} else if(extension_id == 16 /* application_layer_protocol_negotiation (ALPN) */ &&
	          offset + 6 < packet->payload_packet_len) {
	  u_int16_t s_offset = offset+4;
	  u_int16_t tot_alpn_len = ntohs(*((u_int16_t*)&packet->payload[s_offset]));
	  char alpn_str[256];
	  u_int8_t alpn_str_len = 0, i;

#ifdef DEBUG_TLS
	  printf("Server TLS [ALPN: block_len=%u/len=%u]\n", extension_len, tot_alpn_len);
#endif
	  s_offset += 2;
	  tot_alpn_len += s_offset;

	  if(tot_alpn_len > packet->payload_packet_len)
	    return 0;

	  while(s_offset < tot_alpn_len && s_offset < total_len) {
	    u_int8_t alpn_i, alpn_len = packet->payload[s_offset++];

	    if((s_offset + alpn_len) <= tot_alpn_len) {
#ifdef DEBUG_TLS
	      printf("Server TLS [ALPN: %u]\n", alpn_len);
#endif

	      if(((uint32_t)alpn_str_len+alpn_len+1) < (sizeof(alpn_str)-1)) {
	        if(alpn_str_len > 0) {
	          alpn_str[alpn_str_len] = ',';
	          alpn_str_len++;
	        }

	        for(alpn_i=0; alpn_i<alpn_len; alpn_i++)
	        {
	          alpn_str[alpn_str_len+alpn_i] = packet->payload[s_offset+alpn_i];
	        }

	        s_offset += alpn_len, alpn_str_len += alpn_len;;
	      } else {
	        ndpi_set_risk(ndpi_struct, flow, NDPI_TLS_UNCOMMON_ALPN);
	        break;
	      }
	    } else {
	      ndpi_set_risk(ndpi_struct, flow, NDPI_TLS_UNCOMMON_ALPN);
	      break;
	    }
	  } /* while */

	  alpn_str[alpn_str_len] = '\0';

#ifdef DEBUG_TLS
	  printf("Server TLS [ALPN: %s][len: %u]\n", alpn_str, alpn_str_len);
#endif
	  if(ndpi_is_printable_string(alpn_str, alpn_str_len) == 0)
	    ndpi_set_risk(ndpi_struct, flow, NDPI_INVALID_CHARACTERS);

	  if(flow->protos.tls_quic.alpn == NULL)
	    flow->protos.tls_quic.alpn = ndpi_strdup(alpn_str);

	  if(flow->protos.tls_quic.alpn != NULL)
	    tlsCheckUncommonALPN(ndpi_struct, flow);

	  snprintf(ja3.server.alpn, sizeof(ja3.server.alpn), "%s", alpn_str);

	  /* Replace , with - as in JA3 */
	  for(i=0; ja3.server.alpn[i] != '\0'; i++)
	    if(ja3.server.alpn[i] == ',') ja3.server.alpn[i] = '-';
	} else if(extension_id == 11 /* ec_point_formats groups */) {
	  u_int16_t s_offset = offset+4 + 1;

#ifdef DEBUG_TLS
	  printf("Server TLS [EllipticCurveFormat: len=%u]\n", extension_len);
#endif
	  if((s_offset+extension_len-1) <= total_len) {
	    for(i=0; i<extension_len-1 && s_offset+i<packet->payload_packet_len; i++) {
	      u_int8_t s_group = packet->payload[s_offset+i];

#ifdef DEBUG_TLS
	      printf("Server TLS [EllipticCurveFormat: %u]\n", s_group);
#endif

	      if(ja3.server.num_elliptic_curve_point_format < MAX_NUM_JA3)
		ja3.server.elliptic_curve_point_format[ja3.server.num_elliptic_curve_point_format++] = s_group;
	      else {
		invalid_ja3 = 1;
#ifdef DEBUG_TLS
		printf("Server TLS Invalid num elliptic %u\n", ja3.server.num_elliptic_curve_point_format);
#endif
	      }
	    }
	  } else {
	    invalid_ja3 = 1;
#ifdef DEBUG_TLS
	    printf("Server TLS Invalid len %u vs %u\n", s_offset+extension_len, total_len);
#endif
	  }
	}

	i += 4 + extension_len, offset += 4 + extension_len;
      } /* for */

      ja3_str_len = snprintf(ja3_str, JA3_STR_LEN, "%u,", ja3.server.tls_handshake_version);

      for(i=0; (i<ja3.server.num_cipher) && (JA3_STR_LEN > ja3_str_len); i++) {
	rc = snprintf(&ja3_str[ja3_str_len], JA3_STR_LEN-ja3_str_len, "%s%u", (i > 0) ? "-" : "", ja3.server.cipher[i]);

	if(rc <= 0) break; else ja3_str_len += rc;
      }

      if(JA3_STR_LEN > ja3_str_len) {
	rc = snprintf(&ja3_str[ja3_str_len], JA3_STR_LEN-ja3_str_len, ",");
	if(rc > 0 && ja3_str_len + rc < JA3_STR_LEN) ja3_str_len += rc;
      }

      /* ********** */

      for(i=0; (i<ja3.server.num_tls_extension) && (JA3_STR_LEN > ja3_str_len); i++) {
	int rc = snprintf(&ja3_str[ja3_str_len], JA3_STR_LEN-ja3_str_len, "%s%u", (i > 0) ? "-" : "", ja3.server.tls_extension[i]);

	if(rc <= 0) break; else ja3_str_len += rc;
      }

      if(ndpi_struct->enable_ja3_plus) {
	for(i=0; (i<ja3.server.num_elliptic_curve_point_format) && (JA3_STR_LEN > ja3_str_len); i++) {
	  rc = snprintf(&ja3_str[ja3_str_len], JA3_STR_LEN-ja3_str_len, "%s%u",
			(i > 0) ? "-" : "", ja3.server.elliptic_curve_point_format[i]);
	  if((rc > 0) && (ja3_str_len + rc < JA3_STR_LEN)) ja3_str_len += rc; else break;
	}

	if((ja3.server.alpn[0] != '\0') && (JA3_STR_LEN > ja3_str_len)) {
	  rc = snprintf(&ja3_str[ja3_str_len], JA3_STR_LEN-ja3_str_len, ",%s", ja3.server.alpn);
	  if((rc > 0) && (ja3_str_len + rc < JA3_STR_LEN)) ja3_str_len += rc;
	}

#ifdef DEBUG_TLS
	printf("[JA3+] Server: %s \n", ja3_str);
#endif
      } else {
#ifdef DEBUG_TLS
	printf("[JA3] Server: %s \n", ja3_str);
#endif
      }

      ndpi_MD5Init(&ctx);
      ndpi_MD5Update(&ctx, (const unsigned char *)ja3_str, strlen(ja3_str));
      ndpi_MD5Final(md5_hash, &ctx);

      for(i=0, j=0; i<16; i++) {
	int rc = snprintf(&flow->protos.tls_quic.ja3_server[j],
			  sizeof(flow->protos.tls_quic.ja3_server)-j, "%02x", md5_hash[i]);
	if(rc <= 0) break; else j += rc;
      }

#ifdef DEBUG_TLS
      printf("[JA3] Server: %s \n", flow->protos.tls_quic.ja3_server);
#endif
    } else if(handshake_type == 0x01 /* Client Hello */) {
      u_int16_t cipher_len, cipher_offset;
      u_int8_t cookie_len = 0;

      ja3.client.num_cipher = 0;
      ja3.client.num_tls_extension = 0;
      ja3.client.num_elliptic_curve = 0;
      ja3.client.num_elliptic_curve_point_format = 0;
      ja3.client.signature_algorithms[0] = '\0';
      ja3.client.supported_versions[0] = '\0';
      ja3.client.alpn[0] = '\0';

      flow->protos.tls_quic.ssl_version = ja3.client.tls_handshake_version = tls_version;
      if(flow->protos.tls_quic.ssl_version < 0x0303) /* < TLSv1.2 */
	ndpi_set_risk(ndpi_struct, flow, NDPI_TLS_OBSOLETE_VERSION);

      if((session_id_len+base_offset+3) > packet->payload_packet_len)
	return(0); /* Not found */

      if(!isdatagramtls) {
	cipher_len = packet->payload[session_id_len+base_offset+2] + (packet->payload[session_id_len+base_offset+1] << 8);
	cipher_offset = base_offset + session_id_len + 3;
      } else {
	cookie_len = packet->payload[base_offset+session_id_len+1];
#ifdef DEBUG_TLS
	printf("[JA3] Client: DTLS cookie len %d\n", cookie_len);
#endif
	if((session_id_len+base_offset+cookie_len+4) > packet->payload_packet_len)
	  return(0); /* Not found */
	cipher_len = ntohs(*((u_int16_t*)&packet->payload[base_offset+session_id_len+cookie_len+2]));
	cipher_offset = base_offset + session_id_len + cookie_len + 4;
      }

#ifdef DEBUG_TLS
      printf("Client TLS [client cipher_len: %u][tls_version: 0x%04X]\n", cipher_len, tls_version);
#endif

      if((cipher_offset+cipher_len) <= total_len - 1) { /* -1 because variable "id" is a u_int16_t */
	u_int8_t safari_ciphers = 0, chrome_ciphers = 0, this_is_not_safari = 0, looks_like_safari_on_big_sur = 0;

	for(i=0; i<cipher_len;) {
	  u_int16_t *id = (u_int16_t*)&packet->payload[cipher_offset+i];
	  u_int16_t cipher_id = ntohs(*id);

	  if(cipher_offset+i+1 < packet->payload_packet_len &&
	     packet->payload[cipher_offset+i] != packet->payload[cipher_offset+i+1] /* Skip Grease */) {
	    /*
	      Skip GREASE [https://tools.ietf.org/id/draft-ietf-tls-grease-01.html]
	      https://engineering.salesforce.com/tls-fingerprinting-with-ja3-and-ja3s-247362855967
	    */

#if defined(DEBUG_TLS) || defined(DEBUG_HEURISTIC)
	    printf("Client TLS [non-GREASE cipher suite: %u/0x%04X] [%d/%u]\n", cipher_id, cipher_id, i, cipher_len);
#endif

	    if(ja3.client.num_cipher < MAX_NUM_JA3)
	      ja3.client.cipher[ja3.client.num_cipher++] = cipher_id;
	    else {
	      invalid_ja3 = 1;
#ifdef DEBUG_TLS
	      printf("Client TLS Invalid cipher %u\n", ja3.client.num_cipher);
#endif
	    }

#if defined(DEBUG_TLS) || defined(DEBUG_HEURISTIC)
	    printf("Client TLS [cipher suite: %u/0x%04X] [%d/%u]\n", cipher_id, cipher_id, i, cipher_len);
#endif

	    switch(cipher_id) {
	    case TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256:
	    case TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384:
	      safari_ciphers++;
	      break;

	    case TLS_AES_128_GCM_SHA256:
	    case TLS_AES_256_GCM_SHA384:
	    case TLS_CHACHA20_POLY1305_SHA256:
	      chrome_ciphers++;
	      break;

	    case TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256:
	    case TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384:
	    case TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256:
	    case TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256:
	    case TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA:
	    case TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA:
	    case TLS_RSA_WITH_AES_128_CBC_SHA:
	    case TLS_RSA_WITH_AES_256_CBC_SHA:
	    case TLS_RSA_WITH_AES_128_GCM_SHA256:
	    case TLS_RSA_WITH_AES_256_GCM_SHA384:
	      safari_ciphers++, chrome_ciphers++;
	      break;

	    case TLS_RSA_WITH_3DES_EDE_CBC_SHA:
	      looks_like_safari_on_big_sur = 1;
	      break;
	    }
	  } else {
#if defined(DEBUG_TLS) || defined(DEBUG_HEURISTIC)
	    printf("Client TLS [GREASE cipher suite: %u/0x%04X] [%d/%u]\n", cipher_id, cipher_id, i, cipher_len);
#endif

	    this_is_not_safari = 1; /* NOTE: BugSur and up have grease support */
	  }

	  i += 2;
	} /* for */

	/* NOTE:
	   we do not check for duplicates as with signatures because
	   this is time consuming and we want to avoid overhead whem possible
	*/
	if(this_is_not_safari)
	  flow->protos.tls_quic.browser_heuristics.is_safari_tls = 0;
	else if((safari_ciphers == 12) || (this_is_not_safari && looks_like_safari_on_big_sur))
	  flow->protos.tls_quic.browser_heuristics.is_safari_tls = 1;

	if(chrome_ciphers == 13)
	  flow->protos.tls_quic.browser_heuristics.is_chrome_tls = 1;

	/* Note that both Safari and Chrome can overlap */
#ifdef DEBUG_HEURISTIC
	printf("[CIPHERS] [is_chrome_tls: %u (%u)][is_safari_tls: %u (%u)][this_is_not_safari: %u]\n",
	       flow->protos.tls_quic.browser_heuristics.is_chrome_tls,
	       chrome_ciphers,
	       flow->protos.tls_quic.browser_heuristics.is_safari_tls,
	       safari_ciphers,
	       this_is_not_safari);
#endif
      } else {
	invalid_ja3 = 1;
#ifdef DEBUG_TLS
	printf("Client TLS Invalid len %u vs %u\n", (cipher_offset+cipher_len), total_len);
#endif
      }

      offset = base_offset + session_id_len + cookie_len + cipher_len + 2;
      offset += (!isdatagramtls) ? 1 : 2;

      if(offset < total_len) {
	u_int16_t compression_len;
	u_int16_t extensions_len;

	compression_len = packet->payload[offset];
	offset++;

#ifdef DEBUG_TLS
	printf("Client TLS [compression_len: %u]\n", compression_len);
#endif

	// offset += compression_len + 3;
	offset += compression_len;

	if(offset+1 < total_len) {
	  extensions_len = ntohs(*((u_int16_t*)&packet->payload[offset]));
	  offset += 2;

#ifdef DEBUG_TLS
	  printf("Client TLS [extensions_len: %u]\n", extensions_len);
#endif

	  if((extensions_len+offset) <= total_len) {
	    /* Move to the first extension
	       Type is u_int to avoid possible overflow on extension_len addition */
	    u_int extension_offset = 0;

	    while(extension_offset < extensions_len &&
		  offset+extension_offset+4 <= total_len) {
	      u_int16_t extension_id, extension_len, extn_off = offset+extension_offset;


	      extension_id = ntohs(*((u_int16_t*)&packet->payload[offset+extension_offset]));
	      extension_offset += 2;

	      extension_len = ntohs(*((u_int16_t*)&packet->payload[offset+extension_offset]));
	      extension_offset += 2;

#ifdef DEBUG_TLS
	      printf("Client TLS [extension_id: %u][extension_len: %u]\n", extension_id, extension_len);
#endif
	      checkExtensions(ndpi_struct, flow, isdatagramtls,
			      extension_id, extension_len, offset + extension_offset);

	      if(offset + 4 + extension_len > total_len) {
#ifdef DEBUG_TLS
	        printf("[TLS] extension length %u too long (%u, offset %u)\n",
	               extension_len, total_len, offset);
#endif
	        break;
	      }

	      if((extension_id == 0) || (packet->payload[extn_off] != packet->payload[extn_off+1])) {
		/* Skip GREASE */

		if(ja3.client.num_tls_extension < MAX_NUM_JA3)
		  ja3.client.tls_extension[ja3.client.num_tls_extension++] = extension_id;
		else {
		  invalid_ja3 = 1;
#ifdef DEBUG_TLS
		  printf("Client TLS Invalid extensions %u\n", ja3.client.num_tls_extension);
#endif
		}
	      }

	      if(extension_id == 0 /* server name */) {
		u_int16_t len;

#ifdef DEBUG_TLS
		printf("[TLS] Extensions: found server name\n");
#endif
		if((offset+extension_offset+4) < packet->payload_packet_len) {

		  len = (packet->payload[offset+extension_offset+3] << 8) + packet->payload[offset+extension_offset+4];

		  if((offset+extension_offset+5+len) <= packet->payload_packet_len) {

		    char *sni = ndpi_hostname_sni_set(flow, &packet->payload[offset+extension_offset+5], len);
		    int sni_len = strlen(sni);
#ifdef DEBUG_TLS
		    printf("[TLS] SNI: [%s]\n", sni);
#endif
		    if(ndpi_is_printable_string(sni, sni_len) == 0)
		    {
		       ndpi_set_risk(ndpi_struct, flow, NDPI_INVALID_CHARACTERS);
		    }

		    if(!is_quic) {
		      if(ndpi_match_hostname_protocol(ndpi_struct, flow, NDPI_PROTOCOL_TLS, sni, sni_len))
		        flow->protos.tls_quic.subprotocol_detected = 1;
		    } else {
		      if(ndpi_match_hostname_protocol(ndpi_struct, flow, NDPI_PROTOCOL_QUIC, sni, sni_len))
		        flow->protos.tls_quic.subprotocol_detected = 1;
		    }

		    if(ndpi_check_dga_name(ndpi_struct, flow,
					   sni, 1)) {
#ifdef DEBUG_TLS
		      printf("[TLS] SNI: (DGA) [%s]\n", sni);
#endif

		      if((sni_len >= 4)
		         /* Check if it ends in .com or .net */
		         && ((strcmp(&sni[sni_len-4], ".com") == 0) || (strcmp(&sni[sni_len-4], ".net") == 0))
		         && (strncmp(sni, "www.", 4) == 0)) /* Not starting with www.... */
		        ndpi_set_detected_protocol(ndpi_struct, flow, NDPI_PROTOCOL_TOR, NDPI_PROTOCOL_TLS, NDPI_CONFIDENCE_DPI);
		    } else {
#ifdef DEBUG_TLS
		      printf("[TLS] SNI: (NO DGA) [%s]\n", sni);
#endif
		    }
		  } else {
#ifdef DEBUG_TLS
		    printf("[TLS] Extensions server len too short: %u vs %u\n",
			   offset+extension_offset+5+len,
			   packet->payload_packet_len);
#endif
		  }
		}
	      } else if(extension_id == 10 /* supported groups */) {
		u_int16_t s_offset = offset+extension_offset + 2;

#ifdef DEBUG_TLS
		printf("Client TLS [EllipticCurveGroups: len=%u]\n", extension_len);
#endif

		if((s_offset+extension_len-2) <= total_len) {
		  for(i=0; i<(u_int32_t)extension_len-2 && s_offset + i + 1 < total_len; i += 2) {
		    u_int16_t s_group = ntohs(*((u_int16_t*)&packet->payload[s_offset+i]));

#ifdef DEBUG_TLS
		    printf("Client TLS [EllipticCurve: %u/0x%04X]\n", s_group, s_group);
#endif
		    if((s_group == 0) || (packet->payload[s_offset+i] != packet->payload[s_offset+i+1])) {
		      /* Skip GREASE */
		      if(ja3.client.num_elliptic_curve < MAX_NUM_JA3)
			ja3.client.elliptic_curve[ja3.client.num_elliptic_curve++] = s_group;
		      else {
			invalid_ja3 = 1;
#ifdef DEBUG_TLS
			printf("Client TLS Invalid num elliptic %u\n", ja3.client.num_elliptic_curve);
#endif
		      }
		    }
		  }
		} else {
		  invalid_ja3 = 1;
#ifdef DEBUG_TLS
		  printf("Client TLS Invalid len %u vs %u\n", (s_offset+extension_len-1), total_len);
#endif
		}
	      } else // <MASK>

	      extension_offset += extension_len; /* Move to the next extension */

#ifdef DEBUG_TLS
	      printf("Client TLS [extension_offset/len: %u/%u]\n", extension_offset, extension_len);
#endif
	    } /* while */

	    if(!invalid_ja3) {
	      int rc;

	    compute_ja3c:
	      ja3_str_len = snprintf(ja3_str, JA3_STR_LEN, "%u,", ja3.client.tls_handshake_version);

	      for(i=0; i<ja3.client.num_cipher; i++) {
		rc = snprintf(&ja3_str[ja3_str_len], JA3_STR_LEN-ja3_str_len, "%s%u",
			      (i > 0) ? "-" : "", ja3.client.cipher[i]);
		if((rc > 0) && (ja3_str_len + rc < JA3_STR_LEN)) ja3_str_len += rc; else break;
	      }

	      rc = snprintf(&ja3_str[ja3_str_len], JA3_STR_LEN-ja3_str_len, ",");
	      if((rc > 0) && (ja3_str_len + rc < JA3_STR_LEN)) ja3_str_len += rc;

	      /* ********** */

	      for(i=0; i<ja3.client.num_tls_extension; i++) {
		rc = snprintf(&ja3_str[ja3_str_len], JA3_STR_LEN-ja3_str_len, "%s%u",
			      (i > 0) ? "-" : "", ja3.client.tls_extension[i]);
		if((rc > 0) && (ja3_str_len + rc < JA3_STR_LEN)) ja3_str_len += rc; else break;
	      }

	      rc = snprintf(&ja3_str[ja3_str_len], JA3_STR_LEN-ja3_str_len, ",");
	      if((rc > 0) && (ja3_str_len + rc < JA3_STR_LEN)) ja3_str_len += rc;

	      /* ********** */

	      for(i=0; i<ja3.client.num_elliptic_curve; i++) {
		rc = snprintf(&ja3_str[ja3_str_len], JA3_STR_LEN-ja3_str_len, "%s%u",
			      (i > 0) ? "-" : "", ja3.client.elliptic_curve[i]);
		if((rc > 0) && (ja3_str_len + rc < JA3_STR_LEN)) ja3_str_len += rc; else break;
	      }

	      rc = snprintf(&ja3_str[ja3_str_len], JA3_STR_LEN-ja3_str_len, ",");
	      if((rc > 0) && (ja3_str_len + rc < JA3_STR_LEN)) ja3_str_len += rc;

	      for(i=0; i<ja3.client.num_elliptic_curve_point_format; i++) {
		rc = snprintf(&ja3_str[ja3_str_len], JA3_STR_LEN-ja3_str_len, "%s%u",
			      (i > 0) ? "-" : "", ja3.client.elliptic_curve_point_format[i]);
		if((rc > 0) && (ja3_str_len + rc < JA3_STR_LEN)) ja3_str_len += rc; else break;
	      }

	      if(ndpi_struct->enable_ja3_plus) {
		rc = snprintf(&ja3_str[ja3_str_len], JA3_STR_LEN-ja3_str_len,
			      ",%s,%s,%s", ja3.client.signature_algorithms, ja3.client.supported_versions, ja3.client.alpn);
		if((rc > 0) && (ja3_str_len + rc < JA3_STR_LEN)) ja3_str_len += rc;
	      }

#ifdef DEBUG_JA3C
	      printf("[JA3+] Client: %s \n", ja3_str);
#endif

	      ndpi_MD5Init(&ctx);
	      ndpi_MD5Update(&ctx, (const unsigned char *)ja3_str, strlen(ja3_str));
	      ndpi_MD5Final(md5_hash, &ctx);

	      for(i=0, j=0; i<16; i++) {
		rc = snprintf(&flow->protos.tls_quic.ja3_client[j],
			      sizeof(flow->protos.tls_quic.ja3_client)-j, "%02x",
			      md5_hash[i]);
		if(rc > 0) j += rc; else break;
	      }

#ifdef DEBUG_JA3C
	      printf("[JA3] Client: %s \n", flow->protos.tls_quic.ja3_client);
#endif

	      if(ndpi_struct->malicious_ja3_automa.ac_automa != NULL) {
		u_int16_t rc1 = ndpi_match_string(ndpi_struct->malicious_ja3_automa.ac_automa,
						  flow->protos.tls_quic.ja3_client);

		if(rc1 > 0)
		  ndpi_set_risk(ndpi_struct, flow, NDPI_MALICIOUS_JA3);
	      }
	    }

	    /* Before returning to the caller we need to make a final check */
	    if((flow->protos.tls_quic.ssl_version >= 0x0303) /* >= TLSv1.2 */
	       && (flow->protos.tls_quic.alpn == NULL) /* No ALPN */) {
	      ndpi_set_risk(ndpi_struct, flow, NDPI_TLS_NOT_CARRYING_HTTPS);
	    }

	    /* Suspicious Domain Fronting:
	       https://github.com/SixGenInc/Noctilucent/blob/master/docs/ */
	    if(flow->protos.tls_quic.encrypted_sni.esni &&
	       flow->host_server_name[0] != '\0') {
	      ndpi_set_risk(ndpi_struct, flow, NDPI_TLS_SUSPICIOUS_ESNI_USAGE);
	    }

	    /* Add check for missing SNI */
	    if(flow->host_server_name[0] == '\0'
	       && (flow->protos.tls_quic.ssl_version >= 0x0302) /* TLSv1.1 */
	       && (flow->protos.tls_quic.encrypted_sni.esni == NULL) /* No ESNI */
	       ) {
	      /* This is a bit suspicious */
	      ndpi_set_risk(ndpi_struct, flow, NDPI_TLS_MISSING_SNI);
	    }

	    return(2 /* Client Certificate */);
	  } else {
#ifdef DEBUG_TLS
	    printf("[TLS] Client: too short [%u vs %u]\n",
		   (extensions_len+offset), total_len);
#endif
	  }
	} else if(offset == total_len) {
	  /* TLS does not have extensions etc */
	  goto compute_ja3c;
	}
      } else {
#ifdef DEBUG_TLS
	printf("[JA3] Client: invalid length detected\n");
#endif
      }
    }
  }

  return(0); /* Not found */
}