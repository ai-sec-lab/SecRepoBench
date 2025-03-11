static int dissect_softether_host_fqdn(struct ndpi_flow_struct *ndpiflow,
                                       struct ndpi_packet_struct const *packet) {
  u_int8_t const *payload = packet->payload;
  u_int16_t payload_len = packet->payload_packet_len;
  u_int32_t tuple_count;
  // <MASK>
  return 0;
}