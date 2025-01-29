diff = ndpi_min(packet->payload_packet_len-a1, sizeof(flow->initial_binary_bytes));

	if(diff > 0) {
	  memcpy(&flow->initial_binary_bytes, &packet->payload[a1], diff);
	  flow->initial_binary_bytes_len = diff;
	}