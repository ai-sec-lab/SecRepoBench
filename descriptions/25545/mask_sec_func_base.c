int ndpi_check_dga_name(struct ndpi_detection_module_struct *ndpi_str,
			struct ndpi_flow_struct *flow,
			char *name, u_int8_t is_hostname) {
  int len, rc = 0;
  u_int8_t max_num_char_repetitions = 0, last_char = 0, num_char_repetitions = 0, num_dots = 0;
  u_int8_t max_domain_element_len = 0, curr_domain_element_len = 0, first_element_is_numeric = 1;

  // <MASK>

#ifdef DGA_DEBUG
  printf("[DGA] Result: %u", rc);
#endif

  return(rc);
}