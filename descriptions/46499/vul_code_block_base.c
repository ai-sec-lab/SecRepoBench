while(numbers_left) {
	  while(packet_payload[i] >= '0' && packet_payload[i] <= '9') {
	    i++;
	  }

	  if(packet_payload[i++] == ' ') {
	    numbers_left--;
	  }
	  else break;
	}
          
	if(numbers_left) break;
          
	while((packet_payload[i] >= '0' && packet_payload[i] <= '9') ||
	      (packet_payload[i] >= 'A' && packet_payload[i] <= 'Z')) {
	  i++;
	}
          
	if(packet_payload[i] == '\n') {
	  if(++flow->tinc_state > 3) {
	    if(ndpi_struct->tinc_cache == NULL)
	      ndpi_struct->tinc_cache = cache_new(TINC_CACHE_MAX_SIZE);              

	    cache_add(ndpi_struct->tinc_cache, &(flow->tinc_cache_entry), sizeof(flow->tinc_cache_entry));
	    NDPI_LOG_INFO(ndpi_struct, "found tinc tcp connection\n");
	    ndpi_set_detected_protocol(ndpi_struct, flow, NDPI_PROTOCOL_TINC, NDPI_PROTOCOL_UNKNOWN, NDPI_CONFIDENCE_DPI);
	  }
	  return;
	}