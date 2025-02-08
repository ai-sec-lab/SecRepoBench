/* Organization OID: 2.5.29.17 (subjectAltName) */
      u_int8_t matched_name = 0;
      
#ifdef DEBUG_TLS
      printf("******* [TLS] Found subjectAltName\n");
#endif

      i += 3 /* skip the initial patten 55 1D 11 */;
      i++; /* skip the first type, 0x04 == BIT STRING, and jump to it's length */
      i += (packet->payload[i] & 0x80) ? (packet->payload[i] & 0x7F) : 0; /* skip BIT STRING length */
      i += 2; /* skip the second type, 0x30 == SEQUENCE, and jump to it's length */
      i += (packet->payload[i] & 0x80) ? (packet->payload[i] & 0x7F) : 0; /* skip SEQUENCE length */
      i++;

      while(i < packet->payload_packet_len) {
	if(packet->payload[i] == 0x82) {
	  if((i < (packet->payload_packet_len - 1))
	     && ((i + packet->payload[i + 1] + 2) < packet->payload_packet_len)) {
	    u_int8_t len = packet->payload[i + 1];
	    char dNSName[256];

	    i += 2;

	    /* The check "len > sizeof(dNSName) - 1" will be always false. If we add it,
	       the compiler is smart enough to detect it and throws a warning */
	    if(len == 0 /* Looks something went wrong */)
	      break;

	    strncpy(dNSName, (const char*)&packet->payload[i], len);
	    dNSName[len] = '\0';

	    cleanupServerName(dNSName, len);

#if DEBUG_TLS
	    printf("[TLS] dNSName %s [%s]\n", dNSName, flow->protos.stun_ssl.ssl.client_requested_server_name);
#endif
	    if(matched_name == 0) {
	      if((dNSName[0] == '*') && strstr(flow->protos.stun_ssl.ssl.client_requested_server_name, &dNSName[1]))
		matched_name = 1;
	      else if(strcmp(flow->protos.stun_ssl.ssl.client_requested_server_name, dNSName) == 0)
		matched_name = 1;
	    }
	    
	    if(flow->protos.stun_ssl.ssl.server_names == NULL)
	      flow->protos.stun_ssl.ssl.server_names = ndpi_strdup(dNSName),
		flow->protos.stun_ssl.ssl.server_names_len = strlen(dNSName);
	    else {
	      u_int16_t dNSName_len = strlen(dNSName);
	      u_int16_t newstr_len = flow->protos.stun_ssl.ssl.server_names_len + dNSName_len + 1;
	      char *newstr = (char*)ndpi_realloc(flow->protos.stun_ssl.ssl.server_names,
						 flow->protos.stun_ssl.ssl.server_names_len+1, newstr_len+1);

	      if(newstr) {
		flow->protos.stun_ssl.ssl.server_names = newstr;
		flow->protos.stun_ssl.ssl.server_names[flow->protos.stun_ssl.ssl.server_names_len] = ',';
		strncpy(&flow->protos.stun_ssl.ssl.server_names[flow->protos.stun_ssl.ssl.server_names_len+1],
			dNSName, dNSName_len+1);
		flow->protos.stun_ssl.ssl.server_names[newstr_len] = '\0';
		flow->protos.stun_ssl.ssl.server_names_len = newstr_len;
	      }
	    }

	    if(!flow->l4.tcp.tls.subprotocol_detected)
	      if(ndpi_match_hostname_protocol(ndpi_struct, flow, NDPI_PROTOCOL_TLS, dNSName, len))
		flow->l4.tcp.tls.subprotocol_detected = 1;

	    i += len;
	  } else {
#if DEBUG_TLS
	    printf("[TLS] Leftover %u bytes", packet->payload_packet_len - i);
#endif
	    break;
	  }
	} else {
	  break;
	}
      } /* while */

      if(!matched_name)
	NDPI_SET_BIT(flow->risk, NDPI_TLS_CERTIFICATE_MISMATCH); /* Certificate mismatch */