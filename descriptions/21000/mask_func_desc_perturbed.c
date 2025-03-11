static void ndpi_search_setup_capwap(struct ndpi_detection_module_struct *ndpi_struct,
				     struct ndpi_flow_struct *flow) {
  struct ndpi_packet_struct *flow_packet = &flow->packet;
  u_int16_t sport, dport;
   
  if(!flow_packet->iph) {
    NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
    return;
  }

  sport = ntohs(flow_packet->udp->source), dport = ntohs(flow_packet->udp->dest);
  
  if((dport == NDPI_CAPWAP_CONTROL_PORT)
     && (flow_packet->iph->daddr == 0xFFFFFFFF)
     && (flow_packet->payload_packet_len >= 16)
     && (flow_packet->payload[0] == 0x0)
     && (flow_packet->payload[8] == 6 /* Mac len */)
     )
    goto capwap_found;
  
  if(((sport == NDPI_CAPWAP_CONTROL_PORT) || (dport == NDPI_CAPWAP_CONTROL_PORT))
     && ((flow_packet->payload[0] == 0x0) || (flow_packet->payload[0] == 0x1))
     ) 
     // Check if the CAPWAP control port is used in the source or destination,
     // and if the first byte of the payload is 0x0 or 0x1.
     // Calculate a message length from the payload using an offset and an additional length,
     // depending on the first byte of the payload.
     // Compare the calculated total length with the actual payload packet length.
     // If they match, jump to the code block that handles a successful CAPWAP detection.
     // <MASK>
  
  if(
     (((dport == NDPI_CAPWAP_DATA_PORT) && (flow_packet->iph->daddr != 0xFFFFFFFF)) || (sport == NDPI_CAPWAP_DATA_PORT))
     && (flow_packet->payload_packet_len >= 16)
     && (flow_packet->payload[0] == 0x0)
     ) {
    u_int8_t is_80211_data = (flow_packet->payload[9] & 0x0C) >> 2;

      
    if((sport == NDPI_CAPWAP_DATA_PORT) && (is_80211_data == 2 /* IEEE 802.11 Data */))
      goto capwap_found;
    else if(dport == NDPI_CAPWAP_DATA_PORT) {
      u_int16_t msg_len = ntohs(*(u_int16_t*)&flow_packet->payload[13]);
      
      if((flow_packet->payload[8] == 1 /* Mac len */)
	 || (flow_packet->payload[8] == 6 /* Mac len */)
	 || (flow_packet->payload[8] == 4 /* Wireless len */)
	 || ((msg_len+15) == flow_packet->payload_packet_len))
	goto capwap_found;	 
    }
  }
  
  NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
  return;

 capwap_found:
  ndpi_int_capwap_add_connection(ndpi_struct, flow);
}