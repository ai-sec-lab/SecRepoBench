u_int8_t j;

		  s_offset++;
		  
		  // careful not to overflow and loop forever with u_int8_t
		  for(j=0; j+1<version_len; j += 2) {
		    u_int16_t tls_version = ntohs(*((u_int16_t*)&packet->payload[s_offset+j]));
		    u_int8_t unknown_tls_version;
		    
#ifdef DEBUG_TLS
		    printf("Client SSL [TLS version: %s/0x%04X]\n",
			   ndpi_ssl_version2str(tls_version, &unknown_tls_version), tls_version);
#endif

		    if((version_str_len+8) < sizeof(version_str)) {
		      int rc = snprintf(&version_str[version_str_len],
					sizeof(version_str) - version_str_len, "%s%s",
					(version_str_len > 0) ? "," : "",
					ndpi_ssl_version2str(tls_version, &unknown_tls_version));
		      if(rc <= 0)
			break;
		      else
			version_str_len += rc;
		    }
		  }
		if(flow->protos.stun_ssl.ssl.tls_supported_versions == NULL)
		  flow->protos.stun_ssl.ssl.tls_supported_versions = ndpi_strdup(version_str);