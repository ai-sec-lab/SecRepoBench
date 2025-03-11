static void ndpi_check_http_header(struct ndpi_detection_module_struct *ndpi_struct,
				   struct ndpi_flow_struct *ndpiflow) {
  u_int32_t i;
  struct ndpi_packet_struct *packet = &ndpi_struct->packet;

  for(i=0; (i < packet->parsed_lines)
	&& (packet->line[i].ptr != NULL)
	&& (packet->line[i].len > 0); i++) {
    switch(packet->line[i].ptr[0]){
    case 'A':
      if(is_a_suspicious_header(suspicious_http_header_keys_A, packet->line[i])) {
	char str[64];

	snprintf(str, sizeof(str), "Found %.*s", packet->line[i].len, packet->line[i].ptr);
	ndpi_set_risk(ndpi_struct, ndpiflow, NDPI_HTTP_SUSPICIOUS_HEADER, str);
	return;
      }
      break;
    case 'C':
      if(is_a_suspicious_header(suspicious_http_header_keys_C, packet->line[i])) {
	char str[64];

	snprintf(str, sizeof(str), "Found %.*s", packet->line[i].len, packet->line[i].ptr);
	ndpi_set_risk(ndpi_struct, ndpiflow, NDPI_HTTP_SUSPICIOUS_HEADER, str);
	return;
      }
      break;
    case 'M':
      if(is_a_suspicious_header(suspicious_http_header_keys_M, packet->line[i])) {
	char str[64];

	snprintf(str, sizeof(str), "Found %.*s", packet->line[i].len, packet->line[i].ptr);
	ndpi_set_risk(ndpi_struct, ndpiflow, NDPI_HTTP_SUSPICIOUS_HEADER, str);
	return;
      }
      break;
    case 'O':
      if(is_a_suspicious_header(suspicious_http_header_keys_O, packet->line[i])) {
	char str[64];

	snprintf(str, sizeof(str), "Found %.*s", packet->line[i].len, packet->line[i].ptr);
	ndpi_set_risk(ndpi_struct, ndpiflow, NDPI_HTTP_SUSPICIOUS_HEADER, str);
	return;
      }
      break;
    case 'R':
      if(is_a_suspicious_header(suspicious_http_header_keys_R, packet->line[i])) {
	char str[64];

	snprintf(str, sizeof(str), "Found %.*s", packet->line[i].len, packet->line[i].ptr);
	ndpi_set_risk(ndpi_struct, ndpiflow, NDPI_HTTP_SUSPICIOUS_HEADER, str);
	return;
      }
      break;
    case 'S':
      if(is_a_suspicious_header(suspicious_http_header_keys_S, packet->line[i])) {
	char str[64];

	snprintf(str, sizeof(str), "Found %.*s", packet->line[i].len, packet->line[i].ptr);
	ndpi_set_risk(ndpi_struct, ndpiflow, NDPI_HTTP_SUSPICIOUS_HEADER, str);
	return;
      }
      break;
    case 'T':
      if(is_a_suspicious_header(suspicious_http_header_keys_T, packet->line[i])) {
	char str[64];

	snprintf(str, sizeof(str), "Found %.*s", packet->line[i].len, packet->line[i].ptr);
	ndpi_set_risk(ndpi_struct, ndpiflow, NDPI_HTTP_SUSPICIOUS_HEADER, str);
	return;
      }
      break;
    case 'U':
      if(is_a_suspicious_header(suspicious_http_header_keys_U, packet->line[i])) {
	char str[64];

	snprintf(str, sizeof(str), "Found %.*s", packet->line[i].len, packet->line[i].ptr);
	ndpi_set_risk(ndpi_struct, ndpiflow, NDPI_HTTP_SUSPICIOUS_HEADER, str);
	return;
      }
      break;
    case 'X':
      if(is_a_suspicious_header(suspicious_http_header_keys_X, packet->line[i])) {
	char str[64];

	snprintf(str, sizeof(str), "Found %.*s", packet->line[i].len, packet->line[i].ptr);
	ndpi_set_risk(ndpi_struct, ndpiflow, NDPI_HTTP_SUSPICIOUS_HEADER, str);
	return;
      }

      break;
    }
  }
}