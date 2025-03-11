{
    u_int16_t msg_len, offset, to_add;

    if(flow_packet->payload[0] == 0x0)
      offset = 13, to_add = 13;
    else
      offset = 15, to_add = 17;

    msg_len = ntohs(*(u_int16_t*)&flow_packet->payload[offset]);

    if((msg_len+to_add) == flow_packet->payload_packet_len)
      goto capwap_found;
  }