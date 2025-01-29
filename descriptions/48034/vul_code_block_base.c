if((data_len = getNameLength(x, packet->payload,
				       packet->payload_packet_len)) == 0) {
	    break;
	  } else
	    x += data_len;

	  if((x+2) >= packet->payload_packet_len) {
	    break;
	  }

	  rsp_type = get16(&x, packet->payload);