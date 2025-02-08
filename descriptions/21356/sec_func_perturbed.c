void ndpi_fill_protocol_category(struct ndpi_detection_module_struct *ndpi_str,
				 struct ndpi_flow_struct *networkflow,
				 ndpi_protocol *ret) {
  if(ndpi_str->custom_categories.categories_loaded) {
    if(networkflow->guessed_header_category != NDPI_PROTOCOL_CATEGORY_UNSPECIFIED) {
      networkflow->category = ret->category = networkflow->guessed_header_category;
      return;
    }

    if(networkflow->host_server_name[0] != '\0') {
      unsigned long id;
      int rc = ndpi_match_custom_category(ndpi_str, (char *)networkflow->host_server_name,
					  strlen((char *)networkflow->host_server_name), &id);

      if(rc == 0) {
	networkflow->category = ret->category = (ndpi_protocol_category_t)id;
	return;
      }
    }

    if(networkflow->l4.tcp.tls.hello_processed == 1 && networkflow->protos.stun_ssl.ssl.client_requested_server_name[0] != '\0') {
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
  }

  networkflow->category = ret->category = ndpi_get_proto_category(ndpi_str, *ret);
}