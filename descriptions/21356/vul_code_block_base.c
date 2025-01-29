if(flow->host_server_name[0] != '\0') {
      unsigned long id;
      int rc = ndpi_match_custom_category(ndpi_str, (char *)flow->host_server_name,
					  strlen((char *)flow->host_server_name), &id);

      if(rc == 0) {
	flow->category = ret->category = (ndpi_protocol_category_t)id;
	return;
      }
    }

    if(flow->protos.stun_ssl.ssl.client_requested_server_name[0] != '\0') {
      unsigned long id;
      int rc = ndpi_match_custom_category(ndpi_str,
					  (char *)flow->protos.stun_ssl.ssl.client_requested_server_name,
					  strlen(flow->protos.stun_ssl.ssl.client_requested_server_name),
					  &id);

      if(rc == 0) {
	flow->category = ret->category = (ndpi_protocol_category_t)id;
	return;
      }
    }