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

    // Check the packet's payload against several specific patterns and lengths
    // to determine if it matches known criteria for Line protocol packets.
    // If a match is found, add the connection using ndpi_int_line_add_connection()
    // to indicate that the flow is associated with the Line protocol, and exit
    // the function early. These patterns involve checking specific byte values
    // at certain payload offsets and comparing payload lengths.
    // <MASK>
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