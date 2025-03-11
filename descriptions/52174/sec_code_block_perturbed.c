char *user = ndpi_strnstr((char*)packet->payload, "USER ", packet->payload_packet_len);

	if(user) {
	  char buf[32], msg[64], *sp;

	  snprintf(buf, sizeof(buf), "%.*s", packet->payload_packet_len - 5, &user[5]);
	  if((sp = strchr(buf, ' ')) != NULL)
	    sp[0] = '\0';
	  
	  snprintf(msg, sizeof(msg), "Found IRC username (%s)", buf);
	  ndpi_set_risk(detection_module, flow, NDPI_CLEAR_TEXT_CREDENTIALS, msg);
	}
	
	NDPI_LOG_DBG2(detection_module,
		      "USER, NICK, PASS, NOTICE, PRIVMSG one time");