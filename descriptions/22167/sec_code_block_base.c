diff = packet->payload_packet_len - a1;

	if(diff > 0) {
	  diff = ndpi_min(diff, sizeof(flow->initial_binary_bytes));
	  memcpy(&flow->initial_binary_bytes, &packet->payload[a1], diff);
	  flow->initial_binary_bytes_len = diff;
	}