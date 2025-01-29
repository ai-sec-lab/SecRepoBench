u_int16_t msg_len, offset, lengthadjustment;

    if(packet->payload[0] == 0x0)
      offset = 13, lengthadjustment = 13;
    else
      offset = 15, lengthadjustment = 17;

    if (packet->payload_packet_len >= offset + sizeof(u_int16_t)) {
      msg_len = ntohs(*(u_int16_t*)&packet->payload[offset]);

      if((msg_len+lengthadjustment) == packet->payload_packet_len)
        goto capwap_found;
    }