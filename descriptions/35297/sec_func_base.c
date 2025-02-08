int ndpi_match_string_subprotocol(struct ndpi_detection_module_struct *ndpi_str, char *string_to_match,
				  u_int string_to_match_len, ndpi_protocol_match_result *ret_match,
				  u_int8_t is_host_match) {
  ndpi_automa *automa = is_host_match ? &ndpi_str->host_automa : &ndpi_str->content_automa;
  int rc;

  if((automa->ac_automa == NULL) || (string_to_match_len == 0))
    return(NDPI_PROTOCOL_UNKNOWN);

  rc = ndpi_match_string_common(((AC_AUTOMATA_t *) automa->ac_automa),
		  string_to_match,string_to_match_len, &ret_match->protocol_id,
		  &ret_match->protocol_category, &ret_match->protocol_breed);
  return rc < 0 ? NDPI_PROTOCOL_UNKNOWN : ret_match->protocol_id;
}