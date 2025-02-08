void ndpi_fill_protocol_category(struct ndpi_detection_module_struct *ndpi_str,
				 struct ndpi_flow_struct *flow,
				 ndpi_protocol *ret) {
  if(ndpi_str->custom_categories.categories_loaded) {
    if(flow->guessed_header_category != NDPI_PROTOCOL_CATEGORY_UNSPECIFIED) {
      flow->category = ret->category = flow->guessed_header_category;
      return;
    }

    // <MASK>
  }

  flow->category = ret->category = ndpi_get_proto_category(ndpi_str, *ret);
}