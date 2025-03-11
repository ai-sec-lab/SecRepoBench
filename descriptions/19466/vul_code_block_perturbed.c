u_int8_t server_len = packet->payload[i+3];

	    if(packet->payload[i] == 0x55) {
	      certificate_count++;

	      if(certificate_count != 2) continue;
	    }

	    if((server_len+i+3) < packet->payload_packet_len) {
	      char *server_name = (char*)&packet->payload[i+4];
	      u_int8_t begin = 0, len, j, num_dots;

	      while(begin < server_len) {
		if(!ndpi_isprint(server_name[begin]))
		  begin++;
		else
		  break;
	      }

	      // len = ndpi_min(server_len-begin, buffer_len-1);
	      len = buffer_len-1;
	      strncpy(buffer, &server_name[begin], len);
	      buffer[len] = '\0';

	      /* We now have to check if this looks like an IP address or host name */
	      for(j=0, num_dots = 0; j<len; j++) {
		if(!ndpi_isprint((buffer[j]))) {
		  num_dots = 0; /* This is not what we look for */
		  break;
		} else if(buffer[j] == '.') {
		  num_dots++;
		  if(num_dots >=1) break;
		}
	      }

	      if(num_dots >= 1) {
		if(!ndpi_struct->disable_metadata_export) {
		  ndpi_protocol_match_result ret_match;
		  u_int16_t subproto;
		  
		  stripCertificateTrailer(buffer, buffer_len);
		  snprintf(flow->protos.stun_ssl.ssl.server_certificate,
			   sizeof(flow->protos.stun_ssl.ssl.server_certificate), "%s", buffer);
		  
#ifdef DEBUG_TLS
		  printf("[server_certificate: %s]\n", flow->protos.stun_ssl.ssl.server_certificate);
#endif
		  
		  subproto = ndpi_match_host_subprotocol(ndpi_struct, flow,
							 flow->protos.stun_ssl.ssl.server_certificate,
							 strlen(flow->protos.stun_ssl.ssl.server_certificate),
							 &ret_match,
							 NDPI_PROTOCOL_TLS);

		  if(subproto != NDPI_PROTOCOL_UNKNOWN)
		    ndpi_set_detected_protocol(ndpi_struct, flow, subproto, NDPI_PROTOCOL_TLS);
		}

		return(1 /* Server Certificate */);
	      }
	    }