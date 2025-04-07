int getTLScertificate(struct ndpi_detection_module_struct *ndpi_struct,
		      struct ndpi_flow_struct *flow,
		      char *buffer, int buffer_len) {
  struct ndpi_packet_struct *packet = &flow->packet;
  struct ja3_info ja3;
  u_int8_t invalid_ja3 = 0;
  u_int16_t pkt_tls_version = (packet->payload[1] << 8) + packet->payload[2], ja3_str_len;
  char ja3_str[JA3_STR_LEN];
  ndpi_MD5_CTX ctx;
  u_char md5_hash[16];
  int i;

  if(packet->udp) {
    /* Check if this is DTLS or return */
    if((packet->payload[1] != 0xfe)
       || ((packet->payload[2] != 0xff) && (packet->payload[2] != 0xfd))) {
      NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
      return(0);
    }
  }

  flow->protos.stun_ssl.ssl.ssl_version = pkt_tls_version;

  memset(&ja3, 0, sizeof(ja3));

#ifdef DEBUG_TLS
  {
    u_int16_t tls_len = (packet->payload[3] << 8) + packet->payload[4];

    printf("SSL Record [version: 0x%04X][len: %u]\n", pkt_tls_version, tls_len);
  }
#endif

  /*
    Nothing matched so far: let's decode the certificate with some heuristics
    Patches courtesy of Denys Fedoryshchenko <nuclearcat@nuclearcat.com>
  */
  if(packet->payload[0] == 0x16 /* Handshake */) {
    u_int16_t total_len;
    u_int8_t handshake_protocol, header_len;

    if(packet->tcp) {
      header_len = 5; /* SSL Header */
      handshake_protocol = packet->payload[5]; /* handshake protocol a bit misleading, it is message type according TLS specs */
      total_len = (packet->payload[3] << 8) + packet->payload[4];
    } else {
      header_len = 13; /* DTLS header */
      handshake_protocol = packet->payload[13];
      total_len = ntohs(*((u_int16_t*)&packet->payload[11]));
    }

    total_len += header_len;

    memset(buffer, 0, buffer_len);

    /* Truncate total len, search at least in incomplete packet */
    if(total_len > packet->payload_packet_len)
      total_len = packet->payload_packet_len;

    /* At least "magic" 3 bytes, null for string end, otherwise no need to waste cpu cycles */
    if(total_len > 4) {
      u_int16_t base_offset = packet->tcp ? 43 : 59;

#ifdef DEBUG_TLS
      printf("SSL [len: %u][handshake_protocol: %02X]\n", packet->payload_packet_len, handshake_protocol);
#endif

      if((handshake_protocol == 0x02)
	 || (handshake_protocol == 0x0b) /* Server Hello and Certificate message types are interesting for us */) {
	u_int certificate_count = 0;
	u_int16_t tls_version;
	int i;
	
	if(packet->tcp)
	  tls_version = ntohs(*((u_int16_t*)&packet->payload[header_len+4]));
	else
	  tls_version = ntohs(*((u_int16_t*)&packet->payload[header_len+12]));

	ja3.tls_handshake_version = tls_version;

	if(handshake_protocol == 0x02) {
	  u_int16_t offset = base_offset, extension_len, j;
	  u_int8_t  session_id_len = packet->payload[offset];

#ifdef DEBUG_TLS
	  printf("SSL Server Hello [version: 0x%04X]\n", tls_version);
#endif

	  /*
	     The server hello decides about the SSL version of this flow
	     https://networkengineering.stackexchange.com/questions/55752/why-does-wireshark-show-version-tls-1-2-here-instead-of-tls-1-3
	  */
	  flow->protos.stun_ssl.ssl.ssl_version = tls_version;

	  if(packet->udp)
	    offset += 1;
	  else {
	    if(tls_version < 0x7F15 /* TLS 1.3 lacks of session id */)
	      offset += session_id_len+1;
	  }

	  ja3.num_cipher = 1, ja3.cipher[0] = ntohs(*((u_int16_t*)&packet->payload[offset]));
	  flow->protos.stun_ssl.ssl.server_unsafe_cipher = ndpi_is_safe_ssl_cipher(ja3.cipher[0]);
	  flow->protos.stun_ssl.ssl.server_cipher = ja3.cipher[0];

#ifdef DEBUG_TLS
	  printf("TLS [server][session_id_len: %u][cipher: %04X]\n", session_id_len, ja3.cipher[0]);
#endif

	  offset += 2 + 1;

	  if((offset + 1) < packet->payload_packet_len) /* +1 because we are goint to read 2 bytes */
	    extension_len = ntohs(*((u_int16_t*)&packet->payload[offset]));
	  else
	    extension_len = 0;

#ifdef DEBUG_TLS
	  printf("TLS [server][extension_len: %u]\n", extension_len);
#endif
	  offset += 2;

	  for(i=0; i<extension_len; ) {
	    u_int16_t extension_id, extension_len;

	    if(offset >= (packet->payload_packet_len+4)) break;

	    extension_id  = ntohs(*((u_int16_t*)&packet->payload[offset]));
	    extension_len = ntohs(*((u_int16_t*)&packet->payload[offset+2]));

	    if(ja3.num_tls_extension < MAX_NUM_JA3)
	      ja3.tls_extension[ja3.num_tls_extension++] = extension_id;

#ifdef DEBUG_TLS
	    printf("TLS [server][extension_id: %u/0x%04X][len: %u]\n",
		   extension_id, extension_id, extension_len);
#endif

	    if(extension_id == 43 /* supported versions */) {
	      if(extension_len >= 2) {
		u_int16_t tls_version = ntohs(*((u_int16_t*)&packet->payload[offset+4]));

#ifdef DEBUG_TLS
		printf("TLS [server] [TLS version: 0x%04X]\n", tls_version);
#endif
		
		flow->protos.stun_ssl.ssl.ssl_version = tls_version;
	      }
	    }
	    
	    i += 4 + extension_len, offset += 4 + extension_len;
	  }

	  ja3_str_len = snprintf(ja3_str, sizeof(ja3_str), "%u,", ja3.tls_handshake_version);

	  for(i=0; i<ja3.num_cipher; i++) {
	    int rc = snprintf(&ja3_str[ja3_str_len], sizeof(ja3_str)-ja3_str_len, "%s%u", (i > 0) ? "-" : "", ja3.cipher[i]);

	    if(rc <= 0) break; else ja3_str_len += rc;
	  }
	  
	  ja3_str_len += snprintf(&ja3_str[ja3_str_len], sizeof(ja3_str)-ja3_str_len, ",");

	  /* ********** */

	  for(i=0; i<ja3.num_tls_extension; i++) {
	    int rc = snprintf(&ja3_str[ja3_str_len], sizeof(ja3_str)-ja3_str_len, "%s%u", (i > 0) ? "-" : "", ja3.tls_extension[i]);

	    if(rc <= 0) break; else ja3_str_len += rc;
	  }
	  
#ifdef DEBUG_TLS
	  printf("TLS [server] %s\n", ja3_str);
#endif

#ifdef DEBUG_TLS
	  printf("[JA3] Server: %s \n", ja3_str);
#endif

	  ndpi_MD5Init(&ctx);
	  ndpi_MD5Update(&ctx, (const unsigned char *)ja3_str, strlen(ja3_str));
	  ndpi_MD5Final(md5_hash, &ctx);

	  for(i=0, j=0; i<16; i++) {
	    int rc = snprintf(&flow->protos.stun_ssl.ssl.ja3_server[j],
			      sizeof(flow->protos.stun_ssl.ssl.ja3_server)-j, "%02x", md5_hash[i]);
	    if(rc <= 0) break; else j += rc;
	  }
	  
#ifdef DEBUG_TLS
	  printf("[JA3] Server: %s \n", flow->protos.stun_ssl.ssl.ja3_server);
#endif

	  flow->l4.tcp.tls_seen_server_cert = 1;
	} else
	  flow->l4.tcp.tls_seen_certificate = 1;

	/* Check after handshake protocol header (5 bytes) and message header (4 bytes) */
	for(i = 9; i < packet->payload_packet_len-3; i++) {
	  if(((packet->payload[i] == 0x04) && (packet->payload[i+1] == 0x03) && (packet->payload[i+2] == 0x0c))
	     || ((packet->payload[i] == 0x04) && (packet->payload[i+1] == 0x03) && (packet->payload[i+2] == 0x13))
	     || ((packet->payload[i] == 0x55) && (packet->payload[i+1] == 0x04) && (packet->payload[i+2] == 0x03))) {
	    // Extract the server name from the TLS certificate payload.
	    // Check if the extracted server name appears to be a valid host name or IP address.
	    // If valid, store the server name in the flow's `server_certificate` field.
	    // Optionally, attempt to match the extracted server name to a known protocol.
	    // If a subprotocol is detected, set the flow's detected protocol accordingly.
	    // If a valid server name is extracted and optionally matched, return indicating success.
	    // <MASK>
	  }
	}
      } else if(handshake_protocol == 0x01 /* Client Hello */) {
	u_int offset;

#ifdef DEBUG_TLS
	printf("[base_offset: %u][payload_packet_len: %u]\n", base_offset, packet->payload_packet_len);
#endif

	if(base_offset + 2 <= packet->payload_packet_len) {
	  u_int16_t session_id_len;
	  u_int16_t tls_version;

	  if(packet->tcp)
	    tls_version = ntohs(*((u_int16_t*)&packet->payload[header_len+4]));
	  else
	    tls_version = ntohs(*((u_int16_t*)&packet->payload[header_len+12]));

	  session_id_len = packet->payload[base_offset];

	  ja3.tls_handshake_version = tls_version;

	  if((session_id_len+base_offset+2) <= total_len) {
	    u_int16_t cipher_len, cipher_offset;

	    if(packet->tcp) {
	      cipher_len = packet->payload[session_id_len+base_offset+2] + (packet->payload[session_id_len+base_offset+1] << 8);
	      cipher_offset = base_offset + session_id_len + 3;
	    } else {
	      cipher_len = ntohs(*((u_int16_t*)&packet->payload[base_offset+2]));
	      cipher_offset = base_offset+4;
	    }

#ifdef DEBUG_TLS
	    printf("Client SSL [client cipher_len: %u][tls_version: 0x%04X]\n", cipher_len, tls_version);
#endif

	    if((cipher_offset+cipher_len) <= total_len) {
	      for(i=0; i<cipher_len;) {
		u_int16_t *id = (u_int16_t*)&packet->payload[cipher_offset+i];

#ifdef DEBUG_TLS
		printf("Client SSL [cipher suite: %u/0x%04X] [%d/%u]\n", ntohs(*id), ntohs(*id), i, cipher_len);
#endif
		if((*id == 0) || (packet->payload[cipher_offset+i] != packet->payload[cipher_offset+i+1])) {
		  /*
		    Skip GREASE [https://tools.ietf.org/id/draft-ietf-tls-grease-01.html]
		    https://engineering.salesforce.com/tls-fingerprinting-with-ja3-and-ja3s-247362855967
		  */

		  if(ja3.num_cipher < MAX_NUM_JA3)
		    ja3.cipher[ja3.num_cipher++] = ntohs(*id);
		  else {
		    invalid_ja3 = 1;
#ifdef DEBUG_TLS
		    printf("Client SSL Invalid cipher %u\n", ja3.num_cipher);
#endif
		  }
		}

		i += 2;
	      }
	    } else {
	      invalid_ja3 = 1;
#ifdef DEBUG_TLS
	      printf("Client SSL Invalid len %u vs %u\n", (cipher_offset+cipher_len), total_len);
#endif
	    }

	    offset = base_offset + session_id_len + cipher_len + 2;

	    flow->l4.tcp.tls_seen_client_cert = 1;

	    if(offset < total_len) {
	      u_int16_t compression_len;
	      u_int16_t extensions_len;

	      offset += packet->tcp ? 1 : 2;
	      compression_len = packet->payload[offset];
	      offset++;

#ifdef DEBUG_TLS
	      printf("Client SSL [compression_len: %u]\n", compression_len);
#endif

	      // offset += compression_len + 3;
	      offset += compression_len;

	      if(offset < total_len) {
		extensions_len = ntohs(*((u_int16_t*)&packet->payload[offset]));
		offset += 2;

#ifdef DEBUG_TLS
		printf("Client SSL [extensions_len: %u]\n", extensions_len);
#endif

		if((extensions_len+offset) <= total_len) {
		  /* Move to the first extension
		     Type is u_int to avoid possible overflow on extension_len addition */
		  u_int extension_offset = 0;
		  u_int32_t j;

		  while(extension_offset < extensions_len) {
		    u_int16_t extension_id, extension_len, extn_off = offset+extension_offset;

		    extension_id = ntohs(*((u_int16_t*)&packet->payload[offset+extension_offset]));
		    extension_offset += 2;

		    extension_len = ntohs(*((u_int16_t*)&packet->payload[offset+extension_offset]));
		    extension_offset += 2;

#ifdef DEBUG_TLS
		    printf("Client SSL [extension_id: %u][extension_len: %u]\n", extension_id, extension_len);
#endif

		    if((extension_id == 0) || (packet->payload[extn_off] != packet->payload[extn_off+1])) {
		      /* Skip GREASE */

		      if(ja3.num_tls_extension < MAX_NUM_JA3)
			ja3.tls_extension[ja3.num_tls_extension++] = extension_id;
		      else {
			invalid_ja3 = 1;
#ifdef DEBUG_TLS
			printf("Client SSL Invalid extensions %u\n", ja3.num_tls_extension);
#endif
		      }
		    }
		   
		    if(extension_id == 0 /* server name */) {
		      u_int16_t len;

		      len = (packet->payload[offset+extension_offset+3] << 8) + packet->payload[offset+extension_offset+4];
		      len = (u_int)ndpi_min(len, buffer_len-1);

		      if((offset+extension_offset+5+len) < packet->payload_packet_len) {
			strncpy(buffer, (char*)&packet->payload[offset+extension_offset+5], len);
			buffer[len] = '\0';
			
			stripCertificateTrailer(buffer, buffer_len);
			
			if(!ndpi_struct->disable_metadata_export) {
			  snprintf(flow->protos.stun_ssl.ssl.client_certificate,
				   sizeof(flow->protos.stun_ssl.ssl.client_certificate), "%s", buffer);
			}
		      }
		    } else if(extension_id == 10 /* supported groups */) {
		      u_int16_t s_offset = offset+extension_offset + 2;

#ifdef DEBUG_TLS
		      printf("Client SSL [EllipticCurveGroups: len=%u]\n", extension_len);
#endif

		      if((s_offset+extension_len-2) <= total_len) {
			for(i=0; i<extension_len-2;) {
			  u_int16_t s_group = ntohs(*((u_int16_t*)&packet->payload[s_offset+i]));

#ifdef DEBUG_TLS
			  printf("Client SSL [EllipticCurve: %u/0x%04X]\n", s_group, s_group);
#endif
			  if((s_group == 0) || (packet->payload[s_offset+i] != packet->payload[s_offset+i+1])) {
			    /* Skip GREASE */
			    if(ja3.num_elliptic_curve < MAX_NUM_JA3)
			      ja3.elliptic_curve[ja3.num_elliptic_curve++] = s_group;
			    else {
			      invalid_ja3 = 1;
#ifdef DEBUG_TLS
			      printf("Client SSL Invalid num elliptic %u\n", ja3.num_elliptic_curve);
#endif
			    }
			  }

			  i += 2;
			}
		      } else {
			invalid_ja3 = 1;
#ifdef DEBUG_TLS
			printf("Client SSL Invalid len %u vs %u\n", (s_offset+extension_len-1), total_len);
#endif
		      }
		    } else if(extension_id == 11 /* ec_point_formats groups */) {
		      u_int16_t s_offset = offset+extension_offset + 1;

#ifdef DEBUG_TLS
		      printf("Client SSL [EllipticCurveFormat: len=%u]\n", extension_len);
#endif
		      if((s_offset+extension_len) < total_len) {
			for(i=0; i<extension_len-1;i++) {
			  u_int8_t s_group = packet->payload[s_offset+i];

#ifdef DEBUG_TLS
			  printf("Client SSL [EllipticCurveFormat: %u]\n", s_group);
#endif

			  if(ja3.num_elliptic_curve_point_format < MAX_NUM_JA3)
			    ja3.elliptic_curve_point_format[ja3.num_elliptic_curve_point_format++] = s_group;
			  else {
			    invalid_ja3 = 1;
#ifdef DEBUG_TLS
			    printf("Client SSL Invalid num elliptic %u\n", ja3.num_elliptic_curve_point_format);
#endif
			  }
			}
		      } else {
			invalid_ja3 = 1;
#ifdef DEBUG_TLS
			printf("Client SSL Invalid len %u vs %u\n", s_offset+extension_len, total_len);
#endif
		      }
		    } else if(extension_id == 43 /* supported versions */) {
		      u_int8_t version_len = packet->payload[offset+4];
		      
		      if(version_len == (extension_len-1)) {
#ifdef DEBUG_TLS
			u_int8_t j;
			
			for(j=0; j<version_len; j += 2) {
			  u_int16_t tls_version = ntohs(*((u_int16_t*)&packet->payload[offset+5+j]));
			  
			  printf("Client SSL [TLS version: 0x%04X]\n", tls_version);
			}
#endif
		      }
		    }

		    extension_offset += extension_len;

#ifdef DEBUG_TLS
		    printf("Client SSL [extension_offset/len: %u/%u]\n", extension_offset, extension_len);
#endif
		  } /* while */

		  if(!invalid_ja3) {
		  compute_ja3c:
		    ja3_str_len = snprintf(ja3_str, sizeof(ja3_str), "%u,", ja3.tls_handshake_version);

		    for(i=0; i<ja3.num_cipher; i++) {
		      ja3_str_len += snprintf(&ja3_str[ja3_str_len], sizeof(ja3_str)-ja3_str_len, "%s%u",
					      (i > 0) ? "-" : "", ja3.cipher[i]);
		    }

		    ja3_str_len += snprintf(&ja3_str[ja3_str_len], sizeof(ja3_str)-ja3_str_len, ",");

		    /* ********** */

		    for(i=0; i<ja3.num_tls_extension; i++)
		      ja3_str_len += snprintf(&ja3_str[ja3_str_len], sizeof(ja3_str)-ja3_str_len, "%s%u",
					      (i > 0) ? "-" : "", ja3.tls_extension[i]);

		    ja3_str_len += snprintf(&ja3_str[ja3_str_len], sizeof(ja3_str)-ja3_str_len, ",");

		    /* ********** */

		    for(i=0; i<ja3.num_elliptic_curve; i++)
		      ja3_str_len += snprintf(&ja3_str[ja3_str_len], sizeof(ja3_str)-ja3_str_len, "%s%u",
					      (i > 0) ? "-" : "", ja3.elliptic_curve[i]);

		    ja3_str_len += snprintf(&ja3_str[ja3_str_len], sizeof(ja3_str)-ja3_str_len, ",");

		    for(i=0; i<ja3.num_elliptic_curve_point_format; i++)
		      ja3_str_len += snprintf(&ja3_str[ja3_str_len], sizeof(ja3_str)-ja3_str_len, "%s%u",
					      (i > 0) ? "-" : "", ja3.elliptic_curve_point_format[i]);

#ifdef DEBUG_TLS
		    printf("[JA3] Client: %s \n", ja3_str);
#endif

		    ndpi_MD5Init(&ctx);
		    ndpi_MD5Update(&ctx, (const unsigned char *)ja3_str, strlen(ja3_str));
		    ndpi_MD5Final(md5_hash, &ctx);

		    for(i=0, j=0; i<16; i++)
		      j += snprintf(&flow->protos.stun_ssl.ssl.ja3_client[j],
				    sizeof(flow->protos.stun_ssl.ssl.ja3_client)-j, "%02x",
				    md5_hash[i]);

#ifdef DEBUG_TLS
		    printf("[JA3] Client: %s \n", flow->protos.stun_ssl.ssl.ja3_client);
#endif
		  }

		  return(2 /* Client Certificate */);
		}
	      } else if(offset == total_len) {
		/* SSL does not have extensions etc */
		goto compute_ja3c;
	      }
	    }
	  }
	}
      }
    }
  }

  return(0); /* Not found */
}