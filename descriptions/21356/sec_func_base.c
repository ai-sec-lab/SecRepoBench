void ndpi_fill_protocol_category(struct ndpi_detection_module_struct *ndpi_str,
				 struct ndpi_flow_struct *flow,
				 ndpi_protocol *ret) {
  if(ndpi_str->custom_categories.categories_loaded) {
    if(flow->guessed_header_category != NDPI_PROTOCOL_CATEGORY_UNSPECIFIED) {
      flow->category = ret->category = flow->guessed_header_category;
      return;
    }

    if(flow->host_server_name[0] != '\0') {
      unsigned long id;
      int rc = ndpi_match_custom_category(ndpi_str, (char *)flow->host_server_name,
					  strlen((char *)flow->host_server_name), &id);

      if(rc == 0) {
	flow->category = ret->category = (ndpi_protocol_category_t)id;
	return;
      }
    }

    if(flow->l4.tcp.tls.hello_processed == 1 && flow->protos.stun_ssl.ssl.client_requested_server_name[0] != '\0') {
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
  }

  flow->category = ret->category = ndpi_get_proto_category(ndpi_str, *ret);
}