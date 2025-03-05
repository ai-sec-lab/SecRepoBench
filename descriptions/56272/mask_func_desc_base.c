static int dissect_softether_host_fqdn(struct ndpi_flow_struct *flow,
                                       struct ndpi_packet_struct const *packet) {
  u_int8_t const *payload = packet->payload;
  u_int16_t payload_len = packet->payload_packet_len;
  u_int32_t tuple_count;
  // Initialize variables for processing and flags for hostname and FQDN detection.
  // Check if the payload length is sufficient and retrieve the tuple count from the payload.
  // Validate the tuple count and adjust the payload and its length accordingly.
  // Extract the first value from the payload to determine if it contains a "host_name" identifier.
  // Iterate over each tuple in the payload, extracting values and checking their types and identifiers.
  // If a "host_name" is detected, prepare to store its value in the flow structure's hostname field.
  // If a "ddns_fqdn" identifier is detected, prepare to store its value in the flow structure's fqdn field.
  // Continue processing tuples, adjusting the payload and its length after each extraction.
  // If any unprocessed data remains or tuple count is not exhausted, return an error code.
  // <MASK>
  return 0;
}