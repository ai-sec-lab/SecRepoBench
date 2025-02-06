if(get_u_int16_t(packet->payload, a+2) == ntohs(0x0d0a)) {
	/* \r\n\r\n */
	int diff; /* No unsigned ! */
	u_int32_t a1 = a + 4;

	diff = ndpi_min(packet->payload_packet_len-a1, sizeof(flow->initial_binary_bytes));
	
	if(diff > 0) {
	  memcpy(&flow->initial_binary_bytes, &packet->payload[a1], diff);
	  flow->initial_binary_bytes_len = diff;
	}
      }