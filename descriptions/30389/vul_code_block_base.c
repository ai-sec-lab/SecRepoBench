if(tag_offset_start + prev_offset + len > crypto_data_len)
      break;
#if 0
    printf("crypto_data_len %u prev_offset %u offset %u len %d\n",
	   crypto_data_len, prev_offset, offset, len);
#endif
    if((memcmp(tag, "SNI\0", 4) == 0) &&
       (tag_offset_start + prev_offset + len < crypto_data_len)) {
      sni_len = MIN(len, sizeof(flow->protos.tls_quic_stun.tls_quic.client_requested_server_name) - 1);
      memcpy(flow->protos.tls_quic_stun.tls_quic.client_requested_server_name,
             &crypto_data[tag_offset_start + prev_offset], sni_len);
      flow->protos.tls_quic_stun.tls_quic.client_requested_server_name[sni_len] = '\0';

      NDPI_LOG_DBG2(ndpi_struct, "SNI: [%s]\n",
                    flow->protos.tls_quic_stun.tls_quic.client_requested_server_name);

      ndpi_match_host_subprotocol(ndpi_struct, flow,
                                  (char *)flow->protos.tls_quic_stun.tls_quic.client_requested_server_name,
                                  strlen((const char*)flow->protos.tls_quic_stun.tls_quic.client_requested_server_name),
                                  &ret_match, NDPI_PROTOCOL_QUIC);
      flow->protos.tls_quic_stun.tls_quic.hello_processed = 1; /* Allow matching of custom categories */

      ndpi_check_dga_name(ndpi_struct, flow,
                          flow->protos.tls_quic_stun.tls_quic.client_requested_server_name, 1);

      sni_found = 1;
      if (ua_found)
        return;
    }

    if(memcmp(tag, "UAID", 4) == 0) {
      u_int uaid_offset = tag_offset_start + prev_offset;
            
      if((uaid_offset + len) < crypto_data_len) {      
	NDPI_LOG_DBG2(ndpi_struct, "UA: [%.*s]\n", len, &crypto_data[uaid_offset]);
	
	http_process_user_agent(ndpi_struct, flow, &crypto_data[uaid_offset], len); /* http.c */
	ua_found = 1;
	
	if (sni_found)
	  return;
      }
    }