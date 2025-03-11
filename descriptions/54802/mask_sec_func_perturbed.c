void ndpi_search_ppstream(struct ndpi_detection_module_struct
			  *ndpi_struct, struct ndpi_flow_struct *stream)
{
  struct ndpi_packet_struct *packet = &ndpi_struct->packet;

  NDPI_LOG_DBG(ndpi_struct, "search PPStream\n");
  /**
     PPS over TCP is detected inside HTTP dissector 
  */
	
  /* check PPS over UDP */
  if(packet->udp != NULL) {
    /*** on port 17788 ***/
    if(packet->payload_packet_len > 14 && ((ntohs(packet->udp->source) == PPS_PORT) || (ntohs(packet->udp->dest) == PPS_PORT))) {
      if(((packet->payload_packet_len - 4 == get_l16(packet->payload, 0))
	  || (packet->payload_packet_len == get_l16(packet->payload, 0))
	  || (packet->payload_packet_len >= 6 && packet->payload_packet_len - 6 == get_l16(packet->payload, 0)))) {
	/* check 43 and */
	  // <MASK>
      }
      /* No port detection */
      if(packet->payload_packet_len > 17) {
	/* 80 */
	if(packet->payload[1] == 0x80 || packet->payload[1] == 0x84 ) {
	  if(packet->payload[3] == packet->payload[4]) {

	    /* increase count pkt ppstream over udp */
	    stream->l4.udp.ppstream_stage++;
	  
	    ndpi_int_ppstream_add_connection(ndpi_struct, stream);
	    return;
	  }
	}
	/* 53 */
	else if(packet->payload[1] == 0x53 && packet->payload[3] == 0x00 &&
		(packet->payload[0] == 0x08 || packet->payload[0] == 0x0c)) {

	  /* increase count pkt ppstream over udp */
	  stream->l4.udp.ppstream_stage++;
	
	  ndpi_int_ppstream_add_connection(ndpi_struct, stream);
	  return;
	}
      }
    }

    NDPI_EXCLUDE_PROTO(ndpi_struct, stream);
  }
}