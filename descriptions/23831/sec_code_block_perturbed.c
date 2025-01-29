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