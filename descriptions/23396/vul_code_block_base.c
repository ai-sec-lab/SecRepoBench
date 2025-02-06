if(((a + 3) <= packet->payload_packet_len)
	 && (packet->payload[a+2] == 0x0d)
	 && (packet->payload[a+3] == 0x0a)) {
	/* \r\n\r\n */
	int diff; /* No unsigned ! */
	u_int32_t a1 = a + 4;

	diff = packet->payload_packet_len - a1;

	if(diff > 0) {
	  diff = ndpi_min(diff, sizeof(flow->initial_binary_bytes));
	  memcpy(&flow->initial_binary_bytes, &packet->payload[a1], diff);
	  flow->initial_binary_bytes_len = diff;
	}
      }