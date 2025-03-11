static void ndpi_check_spotify(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;
  // const u_int8_t *packet_payload = packet->payload;
  u_int32_t payload_len = packet->payload_packet_len;

  if
  // Check if the packet is a UDP packet, specifically targeting a known Spotify port.
  // If it is, verify the payload for a specific Spotify identifier to recognize Spotify traffic.
  // If recognized, log the detection and mark the flow structure as a Spotify connection, returning early from the function.
  // If the packet is not a UDP packet, check if it is a TCP packet and analyze the payload for specific byte patterns
  // indicative of Spotify, logging and marking the protocol as Spotify if detected.
  // Further, analyze IP packets by checking if the source or destination IP addresses fall within specific known Spotify
  // IP ranges. If they do, log the detection and set the detected protocol as Spotify, then return from the function.
  // <MASK>

  NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
}