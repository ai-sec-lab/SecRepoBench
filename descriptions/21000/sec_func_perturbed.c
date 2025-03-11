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
     ) {
    u_int16_t msg_len, offset, to_add;

    if(flow_packet->payload[0] == 0x0)
      offset = 13, to_add = 13;
    else
      offset = 15, to_add = 17;

    if (flow_packet->payload_packet_len >= offset + sizeof(u_int16_t)) {
      msg_len = ntohs(*(u_int16_t*)&flow_packet->payload[offset]);

      if((msg_len+to_add) == flow_packet->payload_packet_len)
        goto capwap_found;
    }
  }
  
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