if(networkflow->host_server_name[0] != '\0') {
      unsigned long id;
      int rc = ndpi_match_custom_category(ndpi_str, (char *)networkflow->host_server_name,
					  strlen((char *)networkflow->host_server_name), &id);

      if(rc == 0) {
	networkflow->category = ret->category = (ndpi_protocol_category_t)id;
	return;
      }
    }

    if(networkflow->protos.stun_ssl.ssl.client_requested_server_name[0] != '\0') {
      unsigned long id;
      int rc = ndpi_match_custom_category(ndpi_str,
					  (char *)networkflow->protos.stun_ssl.ssl.client_requested_server_name,
					  strlen(networkflow->protos.stun_ssl.ssl.client_requested_server_name),
					  &id);

      if(rc == 0) {
	networkflow->category = ret->category = (ndpi_protocol_category_t)id;
	return;
      }
    }