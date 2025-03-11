void ndpi_search_h323(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *stream)
{
  struct ndpi_packet_struct *packet = &stream->packet;
  u_int16_t dport = 0, sport = 0;

  NDPI_LOG_DBG(ndpi_struct, "search H323\n");

  /*
    The TPKT protocol is used by ISO 8072 (on port 102)
    and H.323. So this check below is to avoid ambiguities
  */
  if((packet->tcp != NULL) && (packet->tcp->dest != ntohs(102))) {
    NDPI_LOG_DBG2(ndpi_struct, "calculated dport over tcp\n");

    /* H323  */
    if(packet->payload_packet_len >= 4
       && (packet->payload[0] == 0x03)
       && (packet->payload[1] == 0x00)) {
      struct tpkt *t = (struct tpkt*)packet->payload;
      u_int16_t len = ntohs(t->len);

      if(packet->payload_packet_len == len) {
	/*
	  We need to check if this packet is in reality
	  a RDP (Remote Desktop) packet encapsulated on TPTK
	*/

	if(packet->payload[4] == (packet->payload_packet_len - sizeof(struct tpkt) - 1)) {
	  /* ISO 8073/X.224 */
	  if((packet->payload[5] == 0xE0 /* CC Connect Request */)
	     || (packet->payload[5] == 0xD0 /* CC Connect Confirm */)) {
	    NDPI_LOG_INFO(ndpi_struct, "found RDP\n");
	    ndpi_set_detected_protocol(ndpi_struct, stream, NDPI_PROTOCOL_RDP, NDPI_PROTOCOL_UNKNOWN);
	    return;
	  }
	}

	stream->l4.tcp.h323_valid_packets++;

	if(stream->l4.tcp.h323_valid_packets >= 2) {
	  NDPI_LOG_INFO(ndpi_struct, "found H323 broadcast\n");
	  ndpi_set_detected_protocol(ndpi_struct, stream, NDPI_PROTOCOL_H323, NDPI_PROTOCOL_UNKNOWN);
	}
      } else {
	/* This is not H.323 */
	NDPI_EXCLUDE_PROTO(ndpi_struct, stream);
	return;
      }
    }
  } else if(packet->udp != NULL) {
    sport = ntohs(packet->udp->source), dport = ntohs(packet->udp->dest);
    NDPI_LOG_DBG2(ndpi_struct, "calculated dport over udp\n");

    if(packet->payload_packet_len >= 6 && packet->payload[0] == 0x80 && packet->payload[1] == 0x08 &&
       (packet->payload[2] == 0xe7 || packet->payload[2] == 0x26) &&
       packet->payload[4] == 0x00 && packet->payload[5] == 0x00)
      {
	NDPI_LOG_INFO(ndpi_struct, "found H323 broadcast\n");
	ndpi_set_detected_protocol(ndpi_struct, stream, NDPI_PROTOCOL_H323, NDPI_PROTOCOL_UNKNOWN);
	return;
      }
    /* H323  */
    if(sport == 1719 || dport == 1719) {
      if((packet->payload_packet_len >= 5)
	 && (packet->payload[0] == 0x16)
	 && (packet->payload[1] == 0x80)
	 && (packet->payload[4] == 0x06)
	 && (packet->payload[5] == 0x00)) {
	NDPI_LOG_INFO(ndpi_struct, "found H323 broadcast\n");
	ndpi_set_detected_protocol(ndpi_struct, stream, NDPI_PROTOCOL_H323, NDPI_PROTOCOL_UNKNOWN);
	return;
      } else if(packet->payload_packet_len >= 20 && packet->payload_packet_len <= 117) {
	NDPI_LOG_INFO(ndpi_struct, "found H323 broadcast\n");
	ndpi_set_detected_protocol(ndpi_struct, stream, NDPI_PROTOCOL_H323, NDPI_PROTOCOL_UNKNOWN);
	return;
      } else {
	NDPI_EXCLUDE_PROTO(ndpi_struct, stream);
	return;
      }
    }
  }
}