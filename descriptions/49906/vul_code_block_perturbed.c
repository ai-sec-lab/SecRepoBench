(http_flow->detected_protocol_stack[1] == NDPI_PROTOCOL_UNKNOWN) {
      /* Avoid putting as subprotocol a "core" protocol such as SSL or DNS */
      if(ndpi_struct->proto_defaults[http_flow->guessed_protocol_id].subprotocol_count == 0) {
	if(http_flow->detected_protocol_stack[0] == NDPI_PROTOCOL_UNKNOWN) {
	  http_flow->detected_protocol_stack[0] = http_flow->guessed_host_protocol_id;
	  http_flow->detected_protocol_stack[1] = http_flow->guessed_protocol_id;
        }
      }
    }
    else {
      if(http_flow->detected_protocol_stack[1] != http_flow->guessed_protocol_id)
	http_flow->guessed_protocol_id = http_flow->detected_protocol_stack[1];
      if(http_flow->detected_protocol_stack[0] != http_flow->guessed_host_protocol_id)
	http_flow->guessed_host_protocol_id = http_flow->detected_protocol_stack[0];
    }