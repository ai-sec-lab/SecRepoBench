if(flow->protos.tls_quic.server_names == NULL)
		      flow->protos.tls_quic.server_names = ndpi_strdup(dNSName),
			flow->protos.tls_quic.server_names_len = strlen(dNSName);
		    else if((u_int16_t)(flow->protos.tls_quic.server_names_len + dNSName_len + 1) > flow->protos.tls_quic.server_names_len) {
		      u_int16_t newstr_len = flow->protos.tls_quic.server_names_len + dNSName_len + 1;
		      char *newstr = (char*)ndpi_realloc(flow->protos.tls_quic.server_names,
							 flow->protos.tls_quic.server_names_len+1, newstr_len+1);

		      if(newstr) {
			flow->protos.tls_quic.server_names = newstr;
			flow->protos.tls_quic.server_names[flow->protos.tls_quic.server_names_len] = ',';
			strncpy(&flow->protos.tls_quic.server_names[flow->protos.tls_quic.server_names_len+1],
				dNSName, dNSName_len+1);
			flow->protos.tls_quic.server_names[newstr_len] = '\0';
			flow->protos.tls_quic.server_names_len = newstr_len;
		      }
		    }