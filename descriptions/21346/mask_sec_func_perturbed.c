void ndpi_search_irc_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;
	
  struct ndpi_id_struct *src_id = flow->src;
  struct ndpi_id_struct *dst = flow->dst;
  int less;
  u_int16_t c = 0;
  u_int16_t c1 = 0;
  u_int16_t port = 0;
  u_int16_t sport = 0;
  u_int16_t dport = 0;
  u_int16_t counter = 0;
  u_int16_t i = 0;
  u_int16_t j = 0;
  u_int16_t k = 0;
  u_int16_t h;
  u_int16_t http_content_ptr_len = 0;
  u_int8_t space = 0;

  NDPI_LOG_DBG(ndpi_struct, "search irc\n");
  if (flow->detected_protocol_stack[0] != NDPI_PROTOCOL_IRC && flow->packet_counter > 70) {
    NDPI_LOG_DBG(ndpi_struct, "exclude irc, packet_counter > 70\n");
    NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_IRC);
    return;
  }
  if (flow->detected_protocol_stack[0] != NDPI_PROTOCOL_IRC && flow->packet_counter > 30 &&
      flow->l4.tcp.irc_stage2 == 0) {
    NDPI_LOG_DBG(ndpi_struct, "exclude irc, packet_counter > 30\n");
    NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_IRC);
    return;
  }
  if (packet->detected_protocol_stack[0] == NDPI_PROTOCOL_IRC) {
    if (src_id != NULL && ((u_int32_t)
			(packet->tick_timestamp - src_id->irc_ts) < ndpi_struct->irc_timeout)) {
      NDPI_LOG_DBG2(ndpi_struct, "irc : save src connection packet detected\n");
      src_id->irc_ts = packet->tick_timestamp;
    } else if (dst != NULL && ((u_int32_t)
			       (packet->tick_timestamp - dst->irc_ts) < ndpi_struct->irc_timeout)) {
      NDPI_LOG_DBG2(ndpi_struct, "irc : save dst connection packet detected\n");
      dst->irc_ts = packet->tick_timestamp;
    }
  }

  if (((dst != NULL && NDPI_COMPARE_PROTOCOL_TO_BITMASK(dst->detected_protocol_bitmask, NDPI_PROTOCOL_IRC)
	&& ((u_int32_t)
	    (packet->tick_timestamp - dst->irc_ts)) <
	ndpi_struct->irc_timeout)) || (src_id != NULL
				       &&
				       NDPI_COMPARE_PROTOCOL_TO_BITMASK
				       (src_id->detected_protocol_bitmask, NDPI_PROTOCOL_IRC)
				       && ((u_int32_t)
					   (packet->tick_timestamp - src_id->irc_ts)) < ndpi_struct->irc_timeout)) {
    if (packet->tcp != NULL) {
      sport = packet->tcp->source;
      dport = packet->tcp->dest;
    }
    if (dst != NULL) {
      for (counter = 0; counter < dst->irc_number_of_port; counter++) {
	if (dst->irc_port[counter] == sport || dst->irc_port[counter] == dport) {
	  dst->last_time_port_used[counter] = packet->tick_timestamp;
	  NDPI_LOG_INFO(ndpi_struct, "found IRC: dest port matched with the DCC port");
	  ndpi_int_irc_add_connection(ndpi_struct, flow);
	  return;
	}
      }
    }
    if (src_id != NULL) {
      for (counter = 0; counter < src_id->irc_number_of_port; counter++) {
	if (src_id->irc_port[counter] == sport || src_id->irc_port[counter] == dport) {
	  src_id->last_time_port_used[counter] = packet->tick_timestamp;
	  NDPI_LOG_INFO(ndpi_struct, "found  IRC: Source port matched with the DCC port");
	  ndpi_int_irc_add_connection(ndpi_struct, flow);
	  return;
	}
      }
    }
  }



  if (flow->detected_protocol_stack[0] != NDPI_PROTOCOL_IRC
      && flow->packet_counter == 2 && (packet->payload_packet_len > 400 && packet->payload_packet_len < 1381)) {
    for (c1 = 50; c1 < packet->payload_packet_len - 23; c1++) {
      if (packet->payload[c1] == 'i' || packet->payload[c1] == 'd') {
	if ((memcmp(&packet->payload[c1], "irc.hackthissite.org0", 21)
	     == 0)
	    || (memcmp(&packet->payload[c1], "irc.gamepad.ca1", 15) == 0)
	    || (memcmp(&packet->payload[c1], "dungeon.axenet.org0", 19)
		== 0)
	    || (memcmp(&packet->payload[c1], "dazed.nuggethaus.net", 20)
		== 0)
	    || (memcmp(&packet->payload[c1], "irc.indymedia.org", 17)
		== 0)
	    || (memcmp(&packet->payload[c1], "irc.cccp-project.net", 20)
		== 0)
	    || (memcmp(&packet->payload[c1], "dirc.followell.net0", 19)
		== 0)
	    || (memcmp(&packet->payload[c1], "irc.discostars.de1", 18)
		== 0)
	    || (memcmp(&packet->payload[c1], "irc.rizon.net", 13) == 0)) {
	  NDPI_LOG_INFO(ndpi_struct,
		   "found IRC SSL: - irc.hackthissite.org0 | irc.gamepad.ca1 | dungeon.axenet.org0 "
		   "| dazed.nuggethaus.net | irc.indymedia.org | irc.discostars.de1 ");
	  ndpi_int_irc_add_connection(ndpi_struct, flow);
	  break;
	}
      }
    }
  }
  if (flow->detected_protocol_stack[0] != NDPI_PROTOCOL_IRC &&
      ndpi_search_irc_ssl_detect_ninety_percent_but_very_fast(ndpi_struct, flow) != 0) {
    return;
  }

  if (flow->detected_protocol_stack[0] != NDPI_PROTOCOL_IRC && flow->packet_counter < 20
      && packet->payload_packet_len >= 8) {
    if (get_u_int8_t(packet->payload, packet->payload_packet_len - 1) == 0x0a
	|| (ntohs(get_u_int16_t(packet->payload, packet->payload_packet_len - 2)) == 0x0a00)) {
      if (memcmp(packet->payload, ":", 1) == 0) {
	if (packet->payload[packet->payload_packet_len - 2] != 0x0d
	    && packet->payload[packet->payload_packet_len - 1] == 0x0a) {
	  ndpi_parse_packet_line_info_any(ndpi_struct, flow);
	} else if (packet->payload[packet->payload_packet_len - 2] == 0x0d) {
	  ndpi_parse_packet_line_info(ndpi_struct, flow);
	} else {
	  flow->l4.tcp.irc_3a_counter++;
	  packet->parsed_lines = 0;
	}
	for (i = 0; i < packet->parsed_lines; i++) {
	  if ((packet->line[i].len > 0) && packet->line[i].ptr[0] == ':') {
	    flow->l4.tcp.irc_3a_counter++;
	    if (flow->l4.tcp.irc_3a_counter == 7) {	/* ':' == 0x3a */
	      NDPI_LOG_INFO(ndpi_struct, "found irc. 0x3a. seven times.");
	      ndpi_int_irc_add_connection(ndpi_struct, flow);
	      goto detected_irc;
	    }
	  }
	}
	if (flow->l4.tcp.irc_3a_counter == 7) {	/* ':' == 0x3a */
	  NDPI_LOG_INFO(ndpi_struct, "found irc. 0x3a. seven times.");
	  ndpi_int_irc_add_connection(ndpi_struct, flow);
	  goto detected_irc;
	}
      }
      if ((memcmp(packet->payload, "USER ", 5) == 0)
	  || (memcmp(packet->payload, "NICK ", 5) == 0)
	  || (memcmp(packet->payload, "PASS ", 5) == 0)
	  || (memcmp(packet->payload, ":", 1) == 0 && ndpi_check_for_NOTICE_or_PRIVMSG(ndpi_struct, flow) != 0)
	  || (memcmp(packet->payload, "PONG ", 5) == 0)
	  || (memcmp(packet->payload, "PING ", 5) == 0)
	  || (memcmp(packet->payload, "JOIN ", 5) == 0)
	  || (memcmp(packet->payload, "NOTICE ", 7) == 0)
	  || (memcmp(packet->payload, "PRIVMSG ", 8) == 0)
	  || (memcmp(packet->payload, "VERSION ", 8) == 0)) {
	NDPI_LOG_DBG2(ndpi_struct,
		 "USER, NICK, PASS, NOTICE, PRIVMSG one time");
	if (flow->l4.tcp.irc_stage == 2) {
	  NDPI_LOG_INFO(ndpi_struct, "found irc");
	  ndpi_int_irc_add_connection(ndpi_struct, flow);
	  flow->l4.tcp.irc_stage = 3;
	}
	if (flow->l4.tcp.irc_stage == 1) {
	  NDPI_LOG_DBG2(ndpi_struct, "second time, stage=2");
	  flow->l4.tcp.irc_stage = 2;
	}
	if (flow->l4.tcp.irc_stage == 0) {
	  NDPI_LOG_DBG2(ndpi_struct, "first time, stage=1");
	  flow->l4.tcp.irc_stage = 1;
	}
	/* irc packets can have either windows line breaks (0d0a) or unix line breaks (0a) */
	if (packet->payload[packet->payload_packet_len - 2] == 0x0d
	    && packet->payload[packet->payload_packet_len - 1] == 0x0a) {
	  ndpi_parse_packet_line_info(ndpi_struct, flow);
	  if (packet->parsed_lines > 1) {
	    NDPI_LOG_DBG2(ndpi_struct, "packet contains more than one line");
	    for (c = 1; c < packet->parsed_lines; c++) {
	      if (packet->line[c].len > 4 && (memcmp(packet->line[c].ptr, "NICK ", 5) == 0
					      || memcmp(packet->line[c].ptr, "USER ", 5) == 0)) {
		NDPI_LOG_INFO(ndpi_struct, "found IRC: two icq signal words in the same packet");
		ndpi_int_irc_add_connection(ndpi_struct, flow);
		flow->l4.tcp.irc_stage = 3;
		return;
	      }
	    }
	  }

	} else if (packet->payload[packet->payload_packet_len - 1] == 0x0a) {
	  ndpi_parse_packet_line_info_any(ndpi_struct, flow);
	  if (packet->parsed_lines > 1) {
	    NDPI_LOG_DBG2(ndpi_struct, "packet contains more than one line");
	    for (c = 1; c < packet->parsed_lines; c++) {
	      if (packet->line[c].len > 4 && (memcmp(packet->line[c].ptr, "NICK ", 5) == 0
						   || memcmp(packet->line[c].ptr, "USER ",
							     5) == 0)) {
		NDPI_LOG_INFO(ndpi_struct, "found IRC: two icq signal words in the same packet");
		ndpi_int_irc_add_connection(ndpi_struct, flow);
		flow->l4.tcp.irc_stage = 3;
		return;
	      }
	    }
	  }
	}
      }
    }
  }

  /**
   * Trying to primarily detect the HTTP Web based IRC chat patterns based on the HTTP headers
   * during the User login time.When the HTTP data gets posted using the POST method ,patterns
   * will be searched in the HTTP content.
   */
  if ((flow->detected_protocol_stack[0] != NDPI_PROTOCOL_IRC) && (flow->l4.tcp.irc_stage == 0)
      && (packet->payload_packet_len > 5)) {
    //HTTP POST Method being employed
    if (memcmp(packet->payload, "POST ", 5) == 0) {
      ndpi_parse_packet_line_info(ndpi_struct, flow);
      if (packet->parsed_lines) {
		  u_int16_t http_header_len = (u_int16_t)((packet->line[packet->parsed_lines - 1].ptr - packet->payload) + 2);
	if (packet->payload_packet_len > http_header_len) {
	  http_content_ptr_len = packet->payload_packet_len - http_header_len;
	}
	if ((ndpi_check_for_IRC_traces(packet->line[0].ptr, packet->line[0].len))
	    || ((packet->http_url_name.ptr)
		&& (ndpi_check_for_IRC_traces(packet->http_url_name.ptr, packet->http_url_name.len)))
	    || ((packet->referer_line.ptr)
		&& (ndpi_check_for_IRC_traces(packet->referer_line.ptr, packet->referer_line.len)))) {
	  NDPI_LOG_DBG2(ndpi_struct,
		   "IRC detected from the Http URL/ Referer header ");
	  flow->l4.tcp.irc_stage = 1;
	  // HTTP POST Request body is not in the same packet.
	  if (!http_content_ptr_len) {
	    return;
	  }
	}
      }
    }
  }

  if ((flow->detected_protocol_stack[0] != NDPI_PROTOCOL_IRC) && (flow->l4.tcp.irc_stage == 1)) {
    if ((((packet->payload_packet_len - http_content_ptr_len) > 10)
	 && (memcmp(packet->payload + http_content_ptr_len, "interface=", 10) == 0)
	 && (ndpi_check_for_Nickname(ndpi_struct, flow) != 0))
	|| (((packet->payload_packet_len - http_content_ptr_len) > 5)
	    && (memcmp(packet->payload + http_content_ptr_len, "item=", 5) == 0)
	    && (ndpi_check_for_cmd(ndpi_struct, flow) != 0))) {
      NDPI_LOG_INFO(ndpi_struct, "found IRC: Nickname, cmd,  one time");
      ndpi_int_irc_add_connection(ndpi_struct, flow);
      return;
    }
  }

 detected_irc:
  NDPI_LOG_DBG2(ndpi_struct, "detected_irc:");

  if (flow->detected_protocol_stack[0] == NDPI_PROTOCOL_IRC) {
    /* maybe this can be deleted at the end */

    if (packet->payload[packet->payload_packet_len - 2] != 0x0d
	&& packet->payload[packet->payload_packet_len - 1] == 0x0a) {
      NDPI_LOG_DBG2(ndpi_struct, "ndpi_parse_packet_line_info_any(ndpi_struct, flow);");
      ndpi_parse_packet_line_info_any(ndpi_struct, flow);
    } else if (packet->payload[packet->payload_packet_len - 2] == 0x0d) {
      ndpi_parse_packet_line_info(ndpi_struct, flow);
    } else {
      return;
    }
    for (i = 0; i < packet->parsed_lines; i++) {
      if (packet->line[i].len > 6 && memcmp(packet->line[i].ptr, "NOTICE ", 7) == 0) {
	NDPI_LOG_DBG2(ndpi_struct, "NOTICE");
	for (j = 7; j < packet->line[i].len - 8; j++) {
	  if (packet->line[i].ptr[j] == ':') {
	    if (memcmp(&packet->line[i].ptr[j + 1], "DCC SEND ", 9) == 0
		|| memcmp(&packet->line[i].ptr[j + 1], "DCC CHAT ", 9) == 0) {
	      NDPI_LOG_INFO(ndpi_struct,
		       "found NOTICE and DCC CHAT or DCC SEND.");
	    }
	  }
	}
      }
      if (packet->payload_packet_len > 0 && packet->payload[0] == 0x3a /* 0x3a = ':' */ ) {
	NDPI_LOG_DBG2(ndpi_struct, "3a");
	for (j = 1; j < packet->line[i].len - 9; j++) {
	  if (packet->line[i].ptr[j] == ' ') {
	    j++;
	    if (packet->line[i].ptr[j] == 'P') {
	      NDPI_LOG_DBG2(ndpi_struct, "P");
	      j++;
	      if (memcmp(&packet->line[i].ptr[j], "RIVMSG ", 7) == 0)
		NDPI_LOG_DBG2(ndpi_struct, "RIVMSG");
	      h = j + 7;
	      goto read_privmsg;
	    }
	  }
	}
      }
      if (packet->line[i].len > 7 && (memcmp(packet->line[i].ptr, "PRIVMSG ", 8) == 0)) {
	NDPI_LOG_DBG2(ndpi_struct, "PRIVMSG	");
	h = 7;
      read_privmsg:
	for (j = h; j < packet->line[i].len - 9; j++) {
	  if (packet->line[i].ptr[j] == ':') {
	    if (memcmp(&packet->line[i].ptr[j + 1], "xdcc ", 5) == 0) {
	      NDPI_LOG_DBG2(ndpi_struct, "xdcc should match.");
	    }
	    j += 2;
	    if (memcmp(&packet->line[i].ptr[j], "DCC ", 4) == 0) {
	      j += 4;
	      NDPI_LOG_DBG2(ndpi_struct, "found DCC.");
	      // <MASK>
	    }
	  }
	}

      }
    }
  }
}