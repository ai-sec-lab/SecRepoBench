static void ndpi_check_spotify(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *stream)
{
  struct ndpi_packet_struct *packet = &stream->packet;
  // const u_int8_t *packet_payload = packet->payload;
  u_int32_t payload_len = packet->payload_packet_len;

  if// <MASK>

  NDPI_EXCLUDE_PROTO(ndpi_struct, stream);
}