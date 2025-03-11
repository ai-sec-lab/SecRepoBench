void ndpi_parse_packet_line_info(struct ndpi_detection_module_struct *ndpi_str, struct ndpi_flow_struct *flow) {
  u_int32_t a;
  struct ndpi_packet_struct *packet = &flow->packet;
    
  if((packet->payload_packet_len < 3) || (packet->payload == NULL))
    return;

  if(packet->packet_lines_parsed_complete != 0)
    return;

  packet->packet_lines_parsed_complete = 1;
  ndpi_reset_packet_line_info(packet);

  packet->line[packet->parsed_lines].ptr = packet->payload;
  packet->line[packet->parsed_lines].len = 0;

  // Parse the payload of the packet to identify and extract lines separated by "\r\n" sequences.
  // For each line, calculate its length and store it in the packet structure.
  // If a sequence "\r\n\r\n" is encountered, extract subsequent bytes and store them in the data flow's initial binary bytes.
  // Analyze the first line to detect and process HTTP response status codes.
  // Identify and extract specific HTTP headers such as "Server:", "Host:", "Content-Type:", etc., storing their positions and lengths.
  // Count the number of headers found in the packet.
  // Mark the position of any empty lines, if encountered.
  // Continue parsing until the end of the payload or the maximum number of lines is reached.
  // <MASK>

  if(packet->parsed_lines >= 1) {
    packet->line[packet->parsed_lines].len =
      (u_int16_t)(((unsigned long) &packet->payload[packet->payload_packet_len]) -
		  ((unsigned long) packet->line[packet->parsed_lines].ptr));
    packet->parsed_lines++;
  }
}