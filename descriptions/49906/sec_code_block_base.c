if(flow->detected_protocol_stack[0] == NDPI_PROTOCOL_UNKNOWN &&
	   flow->guessed_host_protocol_id != NDPI_PROTOCOL_UNKNOWN) {
	  flow->detected_protocol_stack[0] = flow->guessed_host_protocol_id;
	  flow->detected_protocol_stack[1] = flow->guessed_protocol_id;
        }