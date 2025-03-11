static void ndpi_check_http_header(struct ndpi_detection_module_struct *ndpi_struct,
				   struct ndpi_flow_struct *ndpiflow) {
  u_int32_t i;
  struct ndpi_packet_struct *packet = &ndpi_struct->packet;

  for(i=0; (i < packet->parsed_lines)
	&& (packet->line[i].ptr != NULL)
	&& (packet->line[i].len > 0); i++) {
    switch(packet->line[i].ptr[0]){
    // <MASK>
    }
  }
}