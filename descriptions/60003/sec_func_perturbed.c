static void ndpi_search_line(struct ndpi_detection_module_struct *detectionmodulestruct,
                             struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct const * const packet = &detectionmodulestruct->packet;

  NDPI_LOG_DBG(detectionmodulestruct, "searching LineCall\n");

  if(packet->iph && (flow->guessed_protocol_id_by_ip == NDPI_PROTOCOL_LINE)) {
    /*
      The heuristic below (coming from reverse engineering packet traces)
      will apply only to IPv4 and Line IP addresses. This is to avoid puttin
      false positives in other nDPI-decoded protocols.
    */

    if ((packet->payload_packet_len == 110 &&
	 detectionmodulestruct->packet.payload[0] == 0xB6 &&  detectionmodulestruct->packet.payload[1] == 0x18 && detectionmodulestruct->packet.payload[2] == 0x00 && detectionmodulestruct->packet.payload[3] == 0x6A) ||
	(packet->payload_packet_len >= 738 && (detectionmodulestruct->packet.payload[0] == 0xDA || detectionmodulestruct->packet.payload[0] == 0xDB) &&
	 detectionmodulestruct->packet.payload[4] == 0x06 && detectionmodulestruct->packet.payload[5] == 0x02) ||
	(packet->payload_packet_len >= 150 && (detectionmodulestruct->packet.payload[0] == 0xD9 || detectionmodulestruct->packet.payload[0] == 0xD8) &&
	 ((detectionmodulestruct->packet.payload[1] & 0xF0) == 0x90 || (detectionmodulestruct->packet.payload[1] & 0xF0) == 0xD0 || (detectionmodulestruct->packet.payload[1] & 0xF0) == 0xE0) && detectionmodulestruct->packet.payload[4] == 0x06 &&
	 detectionmodulestruct->packet.payload[5] == 0x02)) {
      ndpi_int_line_add_connection(detectionmodulestruct, flow);
      return;
    }

    if ((packet->payload_packet_len == 46 && ntohl(get_u_int32_t(packet->payload, 0)) == 0xb6130006) ||
	(packet->payload_packet_len == 8 && ntohl(get_u_int32_t(packet->payload, 0)) == 0xb6070004) ||
	(packet->payload_packet_len == 16 && ntohl(get_u_int32_t(packet->payload, 0)) == 0xb609000c) ||
	(packet->payload_packet_len >= 2 /* TODO */ && detectionmodulestruct->packet.payload[0] == 0xD0 &&
	 (detectionmodulestruct->packet.payload[1] == 0xB3 || detectionmodulestruct->packet.payload[1] == 0xB4
	  || detectionmodulestruct->packet.payload[1] == 0xDA || detectionmodulestruct->packet.payload[1] == 0xDB))) {
      ndpi_int_line_add_connection(detectionmodulestruct, flow);
      return;
    }
  }

  /* Some "random" UDP packets before the standard RTP stream:
     it seems that the 4th bytes of these packets is some kind of packet
     number. Look for 4 packets per direction with consecutive numbers. */
  if(packet->payload_packet_len > 10) {
    if(flow->l4.udp.line_pkts[packet->packet_direction] == 0) {
      flow->l4.udp.line_base_cnt[packet->packet_direction] = packet->payload[3];
      flow->l4.udp.line_pkts[packet->packet_direction] += 1;
      return;
    } else {
      /* It might be a RTP/RTCP packet. Ignore it and keep looking for the
         LINE packet numbers */
      /* Basic RTP detection */
      if((packet->payload[0] >> 6) == 2 && /* Version 2 */
         (packet->payload[1] == 201 || /* RTCP, Receiver Report */
          packet->payload[1] == 200 || /* RTCP, Sender Report */
          is_valid_rtp_payload_type(packet->payload[1] & 0x7F)) /* RTP */) {
        NDPI_LOG_DBG(detectionmodulestruct, "Probably RTP; keep looking for LINE");
        return;
      } else {
        if((u_int8_t)(flow->l4.udp.line_base_cnt[packet->packet_direction] +
                      flow->l4.udp.line_pkts[packet->packet_direction]) == packet->payload[3]) {
          flow->l4.udp.line_pkts[packet->packet_direction] += 1;
          if(flow->l4.udp.line_pkts[0] >= 4 && flow->l4.udp.line_pkts[1] >= 4) {
            /* To avoid false positives: usually "base pkt numbers" per-direction are different */
            if(flow->l4.udp.line_base_cnt[0] != flow->l4.udp.line_base_cnt[1])
              ndpi_int_line_add_connection(detectionmodulestruct, flow);
            else
              NDPI_EXCLUDE_PROTO(detectionmodulestruct, flow);
	  }
          return;
        }
      }
    }
  }

  NDPI_EXCLUDE_PROTO(detectionmodulestruct, flow);
  return;
}