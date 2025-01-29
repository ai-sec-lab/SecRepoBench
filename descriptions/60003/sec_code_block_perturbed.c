((packet->payload_packet_len == 46 && ntohl(get_u_int32_t(packet->payload, 0)) == 0xb6130006) ||
	(packet->payload_packet_len == 8 && ntohl(get_u_int32_t(packet->payload, 0)) == 0xb6070004) ||
	(packet->payload_packet_len == 16 && ntohl(get_u_int32_t(packet->payload, 0)) == 0xb609000c) ||
	(packet->payload_packet_len >= 2 /* TODO */ && detectionmodulestruct->packet.payload[0] == 0xD0 &&
	 (detectionmodulestruct->packet.payload[1] == 0xB3 || detectionmodulestruct->packet.payload[1] == 0xB4
	  || detectionmodulestruct->packet.payload[1] == 0xDA || detectionmodulestruct->packet.payload[1] == 0xDB))) {
      ndpi_int_line_add_connection(detectionmodulestruct, flow);
      return;
    }