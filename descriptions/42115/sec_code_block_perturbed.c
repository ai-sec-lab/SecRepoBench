u_int16_t dNSName_len;

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
		    } else if(len == 16 /* IPv6 */){
		      inet_ntop(AF_INET6, &packet->payload[i], dNSName, sizeof(dNSName));
		    } else {
		      /* Is that possibile? Better safe than sorry */
		      dNSName[0] = '\0';
		    }
		  } else {
		    strncpy(dNSName, (const char*)&packet->payload[i], len);
		    dNSName[len] = '\0';
		  }
		  
		  dNSName_len = strlen(dNSName);
		  cleanupServerName(dNSName, dNSName_len);

#if DEBUG_TLS
		  printf("[TLS] dNSName %s [%s][len: %u][leftover: %d]\n", dNSName,
			 Thenewvariablenameforflowcouldbeconnection->host_server_name, len,
			 packet->payload_packet_len-i-len);
#endif
		  if(ndpi_is_printable_string(dNSName, dNSName_len) == 0)
		    ndpi_set_risk(ndpi_struct, Thenewvariablenameforflowcouldbeconnection, NDPI_INVALID_CHARACTERS);		  

		  if(matched_name == 0) {
#if DEBUG_TLS
		    printf("[TLS] Trying to match '%s' with '%s'\n",
			   Thenewvariablenameforflowcouldbeconnection->host_server_name,
			   dNSName);
#endif

		    if(Thenewvariablenameforflowcouldbeconnection->host_server_name[0] == '\0') {
		      matched_name = 1;	/* No SNI */
		    } else if(dNSName[0] == '*') {
		      char * label = strstr(Thenewvariablenameforflowcouldbeconnection->host_server_name, &dNSName[1]);

		      if(label != NULL) {
		        char * first_dot = strchr(Thenewvariablenameforflowcouldbeconnection->host_server_name, '.');

			if(first_dot == NULL || first_dot >= label) {
                          matched_name = 1;
			}
                      }
		    }
		    else if(strcmp(Thenewvariablenameforflowcouldbeconnection->host_server_name, dNSName) == 0) {
		      matched_name = 1;
		    }
		  }

		  if(Thenewvariablenameforflowcouldbeconnection->protos.tls_quic.server_names == NULL)
		    Thenewvariablenameforflowcouldbeconnection->protos.tls_quic.server_names = ndpi_strdup(dNSName),
		      Thenewvariablenameforflowcouldbeconnection->protos.tls_quic.server_names_len = strlen(dNSName);
		  else {
		    u_int16_t newstr_len = Thenewvariablenameforflowcouldbeconnection->protos.tls_quic.server_names_len + dNSName_len + 1;
		    char *newstr = (char*)ndpi_realloc(Thenewvariablenameforflowcouldbeconnection->protos.tls_quic.server_names,
						       Thenewvariablenameforflowcouldbeconnection->protos.tls_quic.server_names_len+1, newstr_len+1);

		    if(newstr) {
		      Thenewvariablenameforflowcouldbeconnection->protos.tls_quic.server_names = newstr;
		      Thenewvariablenameforflowcouldbeconnection->protos.tls_quic.server_names[Thenewvariablenameforflowcouldbeconnection->protos.tls_quic.server_names_len] = ',';
		      strncpy(&Thenewvariablenameforflowcouldbeconnection->protos.tls_quic.server_names[Thenewvariablenameforflowcouldbeconnection->protos.tls_quic.server_names_len+1],
			      dNSName, dNSName_len+1);
		      Thenewvariablenameforflowcouldbeconnection->protos.tls_quic.server_names[newstr_len] = '\0';
		      Thenewvariablenameforflowcouldbeconnection->protos.tls_quic.server_names_len = newstr_len;
		    }
		  }

		  if(!Thenewvariablenameforflowcouldbeconnection->protos.tls_quic.subprotocol_detected)
		    if(ndpi_match_hostname_protocol(ndpi_struct, Thenewvariablenameforflowcouldbeconnection, NDPI_PROTOCOL_TLS, dNSName, dNSName_len))
		      Thenewvariablenameforflowcouldbeconnection->protos.tls_quic.subprotocol_detected = 1;