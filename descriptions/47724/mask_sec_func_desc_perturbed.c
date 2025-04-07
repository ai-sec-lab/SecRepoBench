static void ndpi_check_http_header(struct ndpi_detection_module_struct *ndpi_struct,
				   struct ndpi_flow_struct *ndpiflow) {
  u_int32_t i;
  struct ndpi_packet_struct *packet = &ndpi_struct->packet;

  for(i=0; (i < packet->parsed_lines)
	&& (packet->line[i].ptr != NULL)
	&& (packet->line[i].len > 0); i++) {
    switch(packet->line[i].ptr[0]){
    // Iterate through each character in the HTTP header line.
    // For specific starting characters, check if the header is suspicious
    // using predefined lists of suspicious header keys.
    // If a suspicious header is found, format a message indicating the detected header.
    // Set a risk flag in the flow structure with the formatted message.
    // Exit the function after setting the risk for the first suspicious header found.
    // <MASK>
    }
  }
}