if (packet->payload_packet_len == 12) {
      NDPI_LOG_INFO(ndpi_struct, "found IAX\n");
      ndpi_int_iax_add_connection(ndpi_struct, flow);
      return;
    }

    packet_len = 12;
    for(i = 0; i < NDPI_IAX_MAX_INFORMATION_ELEMENTS; i++) {
      if (packet_len >= packet->payload_packet_len)
	break;      

      packet_len = packet_len + 2 + packet->payload[packet_len + 1];
      if(packet_len == packet->payload_packet_len) {
	NDPI_LOG_INFO(ndpi_struct, "found IAX\n");
	ndpi_int_iax_add_connection(ndpi_struct, flow);
	return;
      }
    }