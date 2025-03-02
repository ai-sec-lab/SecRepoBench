(flow->detected_protocol_stack[1] == NDPI_PROTOCOL_UNKNOWN) {
      /* Avoid putting as subprotocol a "core" protocol such as SSL or DNS */
      if(ndpi_struct->proto_defaults[flow->guessed_protocol_id].subprotocol_count == 0) {
	if(flow->detected_protocol_stack[0] == NDPI_PROTOCOL_UNKNOWN) {
	  flow->detected_protocol_stack[0] = flow->guessed_host_protocol_id;
	  flow->detected_protocol_stack[1] = flow->guessed_protocol_id;
        }
      }
    }
    else {
      if(flow->detected_protocol_stack[1] != flow->guessed_protocol_id)
	flow->guessed_protocol_id = flow->detected_protocol_stack[1];
      if(flow->detected_protocol_stack[0] != flow->guessed_host_protocol_id)
	flow->guessed_host_protocol_id = flow->detected_protocol_stack[0];
    }