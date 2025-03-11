if ((packet->payload_packet_len == 110 &&
	 detectionmodulestruct->packet.payload[0] == 0xB6 &&  detectionmodulestruct->packet.payload[1] == 0x18 && detectionmodulestruct->packet.payload[2] == 0x00 && detectionmodulestruct->packet.payload[3] == 0x6A) ||
	(packet->payload_packet_len >= 738 && (detectionmodulestruct->packet.payload[0] == 0xDA || detectionmodulestruct->packet.payload[0] == 0xDB) &&
	 detectionmodulestruct->packet.payload[4] == 0x06 && detectionmodulestruct->packet.payload[5] == 0x02) ||
	(packet->payload_packet_len >= 150 && (detectionmodulestruct->packet.payload[0] == 0xD9 || detectionmodulestruct->packet.payload[0] == 0xD8) &&
	 ((detectionmodulestruct->packet.payload[1] & 0xF0) == 0x90 || (detectionmodulestruct->packet.payload[1] & 0xF0) == 0xD0 || (detectionmodulestruct->packet.payload[1] & 0xF0) == 0xE0) && detectionmodulestruct->packet.payload[4] == 0x06 &&
	 detectionmodulestruct->packet.payload[5] == 0x02)) {
      ndpi_int_line_add_connection(detectionmodulestruct, flow);
      return;
    }

    if ((packet->payload_packet_len == 46 && ntohl(get_u_int32_t(packet->payload, 0)) == 0xb6130006) ||
	(packet->payload_packet_len == 8 && ntohl(get_u_int32_t(packet->payload, 0)) == 0xb6070004) ||
	(packet->payload_packet_len == 16 && ntohl(get_u_int32_t(packet->payload, 0)) == 0xb609000c) ||
	(packet->payload_packet_len >= 2 /* TODO */ && detectionmodulestruct->packet.payload[0] == 0xD0 &&
	 (detectionmodulestruct->packet.payload[1] == 0xB3 || detectionmodulestruct->packet.payload[1] == 0xB4
	  || detectionmodulestruct->packet.payload[1] == 0xDA || detectionmodulestruct->packet.payload[1] == 0xDB))) {
      ndpi_int_line_add_connection(detectionmodulestruct, flow);
      return;
    }