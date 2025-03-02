if ((packet->payload_packet_len == 110 &&
	 ndpi_struct->packet.payload[0] == 0xB6 &&  ndpi_struct->packet.payload[1] == 0x18 && ndpi_struct->packet.payload[2] == 0x00 && ndpi_struct->packet.payload[3] == 0x6A) ||
	(packet->payload_packet_len >= 738 && (ndpi_struct->packet.payload[0] == 0xDA || ndpi_struct->packet.payload[0] == 0xDB) &&
	 ndpi_struct->packet.payload[4] == 0x06 && ndpi_struct->packet.payload[5] == 0x02) ||
	(packet->payload_packet_len >= 150 && (ndpi_struct->packet.payload[0] == 0xD9 || ndpi_struct->packet.payload[0] == 0xD8) &&
	 ((ndpi_struct->packet.payload[1] & 0xF0) == 0x90 || (ndpi_struct->packet.payload[1] & 0xF0) == 0xD0 || (ndpi_struct->packet.payload[1] & 0xF0) == 0xE0) && ndpi_struct->packet.payload[4] == 0x06 &&
	 ndpi_struct->packet.payload[5] == 0x02)) {
      ndpi_int_line_add_connection(ndpi_struct, flow);
      return;
    }

    if ((packet->payload_packet_len == 46 && ntohl(get_u_int32_t(packet->payload, 0)) == 0xb6130006) ||
	(packet->payload_packet_len == 8 && ntohl(get_u_int32_t(packet->payload, 0)) == 0xb6070004) ||
	(packet->payload_packet_len == 16 && ntohl(get_u_int32_t(packet->payload, 0)) == 0xb609000c) ||
	(ndpi_struct->packet.payload[0] == 0xD0 &&
	 (ndpi_struct->packet.payload[1] == 0xB3 || ndpi_struct->packet.payload[1] == 0xB4
	  || ndpi_struct->packet.payload[1] == 0xDA || ndpi_struct->packet.payload[1] == 0xDB))) {
      ndpi_int_line_add_connection(ndpi_struct, flow);
      return;
    }