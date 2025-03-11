
		  i += 2;

		  /* The check "len > sizeof(dNSName) - 1" will be always false. If we add it,
		     the compiler is smart enough to detect it and throws a warning */
		  if((len == 0 /* Looks something went wrong */)
		     || ((i+len) > packet->payload_packet_len))
		    break;

		  if(general_name_type == 0x87) {
		    if(len == 4 /* IPv4 */) {
		      snprintf(dNSName, sizeof(dNSName), "%u.%u.%u.%u",
			       packet->payload[i] & 0xFF,
			       packet->payload[i+1] & 0xFF,
			       packet->payload[i+2] & 0xFF,
			       packet->payload[i+3] & 0xFF);
		    } else {
		      /* 
			 TODO add IPv6 support when able to have 
			 a pcap file for coding
		      */
		    }
		  } else {
		    strncpy(dNSName, (const char*)&packet->payload[i], len);
		    dNSName[len] = '\0';
		  }
		  
		  cleanupServerName(dNSName, len);

#if DEBUG_TLS
		  printf("[TLS] dNSName %s [%s][len: %u][leftover: %d]\n", dNSName,
			 connection->host_server_name, len,
			 packet->payload_packet_len-i-len);
#endif
		  if(ndpi_is_printable_string(dNSName, len) == 0)
		    ndpi_set_risk(ndpi_struct, connection, NDPI_INVALID_CHARACTERS);		  

		  if(matched_name == 0) {
#if DEBUG_TLS
		    printf("[TLS] Trying to match '%s' with '%s'\n",
			   connection->host_server_name,
			   dNSName);
#endif

		    if(connection->host_server_name[0] == '\0') {
		      matched_name = 1;	/* No SNI */
		    } else if(dNSName[0] == '*') {
		      char * label = strstr(connection->host_server_name, &dNSName[1]);

		      if(label != NULL) {
		        char * first_dot = strchr(connection->host_server_name, '.');

			if(first_dot == NULL || first_dot >= label) {
                          matched_name = 1;
			}
                      }
		    }
		    else if(strcmp(connection->host_server_name, dNSName) == 0) {
		      matched_name = 1;
		    }
		  }

		  if(connection->protos.tls_quic.server_names == NULL)
		    connection->protos.tls_quic.server_names = ndpi_strdup(dNSName),
		      connection->protos.tls_quic.server_names_len = strlen(dNSName);
		  else {
		    u_int16_t dNSName_len = strlen(dNSName);
		    u_int16_t newstr_len = connection->protos.tls_quic.server_names_len + dNSName_len + 1;
		    char *newstr = (char*)ndpi_realloc(connection->protos.tls_quic.server_names,
						       connection->protos.tls_quic.server_names_len+1, newstr_len+1);

		    if(newstr) {
		      connection->protos.tls_quic.server_names = newstr;
		      connection->protos.tls_quic.server_names[connection->protos.tls_quic.server_names_len] = ',';
		      strncpy(&connection->protos.tls_quic.server_names[connection->protos.tls_quic.server_names_len+1],
			      dNSName, dNSName_len+1);
		      connection->protos.tls_quic.server_names[newstr_len] = '\0';
		      connection->protos.tls_quic.server_names_len = newstr_len;
		    }
		  }

		  if(!connection->protos.tls_quic.subprotocol_detected)
		    if(ndpi_match_hostname_protocol(ndpi_struct, connection, NDPI_PROTOCOL_TLS, dNSName, len))
		      connection->protos.tls_quic.subprotocol_detected = 1;