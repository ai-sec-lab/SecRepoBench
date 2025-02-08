int ndpi_match_string_subprotocol(struct ndpi_detection_module_struct *ndpi_str, char *string_to_match,
				  u_int string_to_match_len, ndpi_protocol_match_result *ret_match,
				  u_int8_t is_host_match) {
  ndpi_automa *automa = is_host_match ? &ndpi_str->host_automa : &ndpi_str->content_automa;
  int matchresult;

  if((automa->ac_automa == NULL) || (string_to_match_len == 0))
    return(NDPI_PROTOCOL_UNKNOWN);

  // <MASK>
}