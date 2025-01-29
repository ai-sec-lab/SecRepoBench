rc = ndpi_match_string_common(((AC_AUTOMATA_t *) automa->ac_automa),
		  string_to_match,string_to_match_len, &ret_match->protocol_id,
		  &ret_match->protocol_category, &ret_match->protocol_breed);
  return rc < 0 ? rc : ret_match->protocol_id;