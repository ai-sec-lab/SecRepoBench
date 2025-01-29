char buf[32], msg[64], *sp;

	  snprintf(buf, sizeof(buf), "%.*s", packet->payload_packet_len - 5, &user[5]);
	  if((sp = strchr(buf, ' ')) != NULL)
	    sp[0] = '\0';
	  
	  snprintf(msg, sizeof(msg), "Found IRC username (%s)", buf);
	  ndpi_set_risk(ndpi_struct, flow, NDPI_CLEAR_TEXT_CREDENTIALS, msg);