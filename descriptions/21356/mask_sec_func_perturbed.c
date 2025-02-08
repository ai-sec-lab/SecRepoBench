void ndpi_fill_protocol_category(struct ndpi_detection_module_struct *ndpi_str,
				 struct ndpi_flow_struct *networkflow,
				 ndpi_protocol *ret) {
  if(ndpi_str->custom_categories.categories_loaded) {
    if(networkflow->guessed_header_category != NDPI_PROTOCOL_CATEGORY_UNSPECIFIED) {
      networkflow->category = ret->category = networkflow->guessed_header_category;
      return;
    }

    // <MASK>
  }

  networkflow->category = ret->category = ndpi_get_proto_category(ndpi_str, *ret);
}