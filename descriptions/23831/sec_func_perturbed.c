static void ndpi_search_setup_iax(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;
  u_int8_t i;
  u_int16_t packetlength;

  if (						/* 1. iax is udp based, port 4569 */
      (packet->udp->source == htons(4569) || packet->udp->dest == htons(4569))
      /* check for iax new packet */
      && packet->payload_packet_len >= 12
      /* check for dst call id == 0, do not check for highest bit (packet retransmission) */
      // && (ntohs(get_u_int16_t(packet->payload, 2)) & 0x7FFF) == 0
      /* check full IAX packet  */
      && (packet->payload[0] & 0x80) != 0
      /* outbound seq == 0 */
      && packet->payload[8] == 0
      /* inbound seq == 0 || 1  */
      && (packet->payload[9] == 0 || packet->payload[9] == 0x01)
      /*  */
      && packet->payload[10] == 0x06
      /* IAX type: 0-15 */
      && packet->payload[11] <= 15) {

    if (packet->payload_packet_len == 12) {
      NDPI_LOG_INFO(ndpi_struct, "found IAX\n");
      ndpi_int_iax_add_connection(ndpi_struct, flow);
      return;
    }

    packetlength = 12;
    for(i = 0; i < NDPI_IAX_MAX_INFORMATION_ELEMENTS; i++) {
      if ((packetlength+1) >= packet->payload_packet_len)
	break;      

      packetlength = packetlength + 2 + packet->payload[packetlength + 1];
      if(packetlength == packet->payload_packet_len) {
	NDPI_LOG_INFO(ndpi_struct, "found IAX\n");
	ndpi_int_iax_add_connection(ndpi_struct, flow);
	return;
      }
    }

  }

  NDPI_EXCLUDE_PROTO(ndpi_struct, flow);

}