void ndpi_search_mail_imap_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *flow_packet = &flow->packet;       
  u_int16_t i = 0;
  u_int16_t space_pos = 0;
  u_int16_t command_start = 0;
  u_int8_t saw_command = 0;
  /* const u_int8_t *command = 0; */

  NDPI_LOG_DBG(ndpi_struct, "search IMAP_IMAP\n");

#ifdef IMAP_DEBUG
  printf("%s() [%s]\n", __FUNCTION__, flow_packet->payload);
#endif

  if(flow->l4.tcp.mail_imap_starttls == 2) {
    NDPI_LOG_DBG2(ndpi_struct, "starttls detected\n");
    NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_MAIL_IMAP);
    NDPI_DEL_PROTOCOL_FROM_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_TLS);
    return;
  }

  if(flow_packet->payload_packet_len >= 4 && ntohs(get_u_int16_t(flow_packet->payload, flow_packet->payload_packet_len - 2)) == 0x0d0a) {
    // the DONE command appears without a tag
    if(flow_packet->payload_packet_len == 6 && ((flow_packet->payload[0] == 'D' || flow_packet->payload[0] == 'd')
					    && (flow_packet->payload[1] == 'O' || flow_packet->payload[1] == 'o')
					    && (flow_packet->payload[2] == 'N' || flow_packet->payload[2] == 'n')
					    && (flow_packet->payload[3] == 'E' || flow_packet->payload[3] == 'e'))) {
      flow->l4.tcp.mail_imap_stage += 1;
      saw_command = 1;
    } else {

      if(flow->l4.tcp.mail_imap_stage < 4) {
	// search for the first space character (end of the tag)
	while (i < 20 && i < flow_packet->payload_packet_len) {
	  if(i > 0 && flow_packet->payload[i] == ' ') {
	    space_pos = i;
	    break;
	  }
	  if(!((flow_packet->payload[i] >= 'a' && flow_packet->payload[i] <= 'z') ||
		(flow_packet->payload[i] >= 'A' && flow_packet->payload[i] <= 'Z') ||
		(flow_packet->payload[i] >= '0' && flow_packet->payload[i] <= '9') || flow_packet->payload[i] == '*' || flow_packet->payload[i] == '.')) {
	    goto imap_excluded;
	  }
	  i++;
	}
	if(space_pos == 0 || space_pos == (flow_packet->payload_packet_len - 1)) {
	  goto imap_excluded;
	}
	// now walk over a possible mail number to the next space
	i++;
	if(i < flow_packet->payload_packet_len && (flow_packet->payload[i] >= '0' && flow_packet->payload[i] <= '9')) {
	  while (i < 20 && i < flow_packet->payload_packet_len) {
	    if(i > 0 && flow_packet->payload[i] == ' ') {
	      space_pos = i;
	      break;
	    }
	    if(!(flow_packet->payload[i] >= '0' && flow_packet->payload[i] <= '9')) {
	      goto imap_excluded;
	    }
	    i++;
	  }
	  if(space_pos == 0 || space_pos == (flow_packet->payload_packet_len - 1)) {
	    goto imap_excluded;
	  }
	}
	command_start = space_pos + 1;
	/* command = &(packet->payload[command_start]); */
      } else {
	command_start = 0;
	/* command = &(packet->payload[command_start]); */
      }

      if((command_start + 3) < flow_packet->payload_packet_len) {
	if((flow_packet->payload[command_start] == 'O' || flow_packet->payload[command_start] == 'o')
	    && (flow_packet->payload[command_start + 1] == 'K' || flow_packet->payload[command_start + 1] == 'k')
	    && flow_packet->payload[command_start + 2] == ' ') {
	  flow->l4.tcp.mail_imap_stage += 1;
	  if(flow->l4.tcp.mail_imap_starttls == 1)
	    flow->l4.tcp.mail_imap_starttls = 2;
	  saw_command = 1;
	} else if((flow_packet->payload[command_start] == 'U' || flow_packet->payload[command_start] == 'u')
		   && (flow_packet->payload[command_start + 1] == 'I' || flow_packet->payload[command_start + 1] == 'i')
		   && (flow_packet->payload[command_start + 2] == 'D' || flow_packet->payload[command_start + 2] == 'd')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	}
      }
      if((command_start + 10) < flow_packet->payload_packet_len) {
	if((flow_packet->payload[command_start] == 'C' || flow_packet->payload[command_start] == 'c')
	    && (flow_packet->payload[command_start + 1] == 'A' || flow_packet->payload[command_start + 1] == 'a')
	    && (flow_packet->payload[command_start + 2] == 'P' || flow_packet->payload[command_start + 2] == 'p')
	    && (flow_packet->payload[command_start + 3] == 'A' || flow_packet->payload[command_start + 3] == 'a')
	    && (flow_packet->payload[command_start + 4] == 'B' || flow_packet->payload[command_start + 4] == 'b')
	    && (flow_packet->payload[command_start + 5] == 'I' || flow_packet->payload[command_start + 5] == 'i')
	    && (flow_packet->payload[command_start + 6] == 'L' || flow_packet->payload[command_start + 6] == 'l')
	    && (flow_packet->payload[command_start + 7] == 'I' || flow_packet->payload[command_start + 7] == 'i')
	    && (flow_packet->payload[command_start + 8] == 'T' || flow_packet->payload[command_start + 8] == 't')
	    && (flow_packet->payload[command_start + 9] == 'Y' || flow_packet->payload[command_start + 9] == 'y')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	}
      }
      if((command_start + 8) < flow_packet->payload_packet_len) {
	if((flow_packet->payload[command_start] == 'S' || flow_packet->payload[command_start] == 's')
	    && (flow_packet->payload[command_start + 1] == 'T' || flow_packet->payload[command_start + 1] == 't')
	    && (flow_packet->payload[command_start + 2] == 'A' || flow_packet->payload[command_start + 2] == 'a')
	    && (flow_packet->payload[command_start + 3] == 'R' || flow_packet->payload[command_start + 3] == 'r')
	    && (flow_packet->payload[command_start + 4] == 'T' || flow_packet->payload[command_start + 4] == 't')
	    && (flow_packet->payload[command_start + 5] == 'T' || flow_packet->payload[command_start + 5] == 't')
	    && (flow_packet->payload[command_start + 6] == 'L' || flow_packet->payload[command_start + 6] == 'l')
	    && (flow_packet->payload[command_start + 7] == 'S' || flow_packet->payload[command_start + 7] == 's')) {
        flow->l4.tcp.mail_imap_stage += 1;
        flow->l4.tcp.mail_imap_starttls = 1;
        flow->detected_protocol_stack[0] = NDPI_PROTOCOL_MAIL_IMAPS;
        saw_command = 1;
	}
      }
      if((command_start + 5) < flow_packet->payload_packet_len) {
	if((flow_packet->payload[command_start] == 'L' || flow_packet->payload[command_start] == 'l')
	    && (flow_packet->payload[command_start + 1] == 'O' || flow_packet->payload[command_start + 1] == 'o')
	    && (flow_packet->payload[command_start + 2] == 'G' || flow_packet->payload[command_start + 2] == 'g')
	    && (flow_packet->payload[command_start + 3] == 'I' || flow_packet->payload[command_start + 3] == 'i')
	    && (flow_packet->payload[command_start + 4] == 'N' || flow_packet->payload[command_start + 4] == 'n')) {
	  // <MASK>
	  
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	} else if((flow_packet->payload[command_start] == 'F' || flow_packet->payload[command_start] == 'f')
		   && (flow_packet->payload[command_start + 1] == 'E' || flow_packet->payload[command_start + 1] == 'e')
		   && (flow_packet->payload[command_start + 2] == 'T' || flow_packet->payload[command_start + 2] == 't')
		   && (flow_packet->payload[command_start + 3] == 'C' || flow_packet->payload[command_start + 3] == 'c')
		   && (flow_packet->payload[command_start + 4] == 'H' || flow_packet->payload[command_start + 4] == 'h')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	} else if((flow_packet->payload[command_start] == 'F' || flow_packet->payload[command_start] == 'f')
		   && (flow_packet->payload[command_start + 1] == 'L' || flow_packet->payload[command_start + 1] == 'l')
		   && (flow_packet->payload[command_start + 2] == 'A' || flow_packet->payload[command_start + 2] == 'a')
		   && (flow_packet->payload[command_start + 3] == 'G' || flow_packet->payload[command_start + 3] == 'g')
		   && (flow_packet->payload[command_start + 4] == 'S' || flow_packet->payload[command_start + 4] == 's')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	} else if((flow_packet->payload[command_start] == 'C' || flow_packet->payload[command_start] == 'c')
		   && (flow_packet->payload[command_start + 1] == 'H' || flow_packet->payload[command_start + 1] == 'h')
		   && (flow_packet->payload[command_start + 2] == 'E' || flow_packet->payload[command_start + 2] == 'e')
		   && (flow_packet->payload[command_start + 3] == 'C' || flow_packet->payload[command_start + 3] == 'c')
		   && (flow_packet->payload[command_start + 4] == 'K' || flow_packet->payload[command_start + 4] == 'k')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	} else if((flow_packet->payload[command_start] == 'S' || flow_packet->payload[command_start] == 's')
		   && (flow_packet->payload[command_start + 1] == 'T' || flow_packet->payload[command_start + 1] == 't')
		   && (flow_packet->payload[command_start + 2] == 'O' || flow_packet->payload[command_start + 2] == 'o')
		   && (flow_packet->payload[command_start + 3] == 'R' || flow_packet->payload[command_start + 3] == 'r')
		   && (flow_packet->payload[command_start + 4] == 'E' || flow_packet->payload[command_start + 4] == 'e')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	}
      }
      if((command_start + 12) < flow_packet->payload_packet_len) {
	if((flow_packet->payload[command_start] == 'A' || flow_packet->payload[command_start] == 'a')
	    && (flow_packet->payload[command_start + 1] == 'U' || flow_packet->payload[command_start + 1] == 'u')
	    && (flow_packet->payload[command_start + 2] == 'T' || flow_packet->payload[command_start + 2] == 't')
	    && (flow_packet->payload[command_start + 3] == 'H' || flow_packet->payload[command_start + 3] == 'h')
	    && (flow_packet->payload[command_start + 4] == 'E' || flow_packet->payload[command_start + 4] == 'e')
	    && (flow_packet->payload[command_start + 5] == 'N' || flow_packet->payload[command_start + 5] == 'n')
	    && (flow_packet->payload[command_start + 6] == 'T' || flow_packet->payload[command_start + 6] == 't')
	    && (flow_packet->payload[command_start + 7] == 'I' || flow_packet->payload[command_start + 7] == 'i')
	    && (flow_packet->payload[command_start + 8] == 'C' || flow_packet->payload[command_start + 8] == 'c')
	    && (flow_packet->payload[command_start + 9] == 'A' || flow_packet->payload[command_start + 9] == 'a')
	    && (flow_packet->payload[command_start + 10] == 'T' || flow_packet->payload[command_start + 10] == 't')
	    && (flow_packet->payload[command_start + 11] == 'E' || flow_packet->payload[command_start + 11] == 'e')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	}
      }
      if((command_start + 9) < flow_packet->payload_packet_len) {
	if((flow_packet->payload[command_start] == 'N' || flow_packet->payload[command_start] == 'n')
	    && (flow_packet->payload[command_start + 1] == 'A' || flow_packet->payload[command_start + 1] == 'a')
	    && (flow_packet->payload[command_start + 2] == 'M' || flow_packet->payload[command_start + 2] == 'm')
	    && (flow_packet->payload[command_start + 3] == 'E' || flow_packet->payload[command_start + 3] == 'e')
	    && (flow_packet->payload[command_start + 4] == 'S' || flow_packet->payload[command_start + 4] == 's')
	    && (flow_packet->payload[command_start + 5] == 'P' || flow_packet->payload[command_start + 5] == 'p')
	    && (flow_packet->payload[command_start + 6] == 'A' || flow_packet->payload[command_start + 6] == 'a')
	    && (flow_packet->payload[command_start + 7] == 'C' || flow_packet->payload[command_start + 7] == 'c')
	    && (flow_packet->payload[command_start + 8] == 'E' || flow_packet->payload[command_start + 8] == 'e')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	}
      }
      if((command_start + 4) < flow_packet->payload_packet_len) {
	if((flow_packet->payload[command_start] == 'L' || flow_packet->payload[command_start] == 'l')
	    && (flow_packet->payload[command_start + 1] == 'S' || flow_packet->payload[command_start + 1] == 's')
	    && (flow_packet->payload[command_start + 2] == 'U' || flow_packet->payload[command_start + 2] == 'u')
	    && (flow_packet->payload[command_start + 3] == 'B' || flow_packet->payload[command_start + 3] == 'b')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	} else if((flow_packet->payload[command_start] == 'L' || flow_packet->payload[command_start] == 'l')
		   && (flow_packet->payload[command_start + 1] == 'I' || flow_packet->payload[command_start + 1] == 'i')
		   && (flow_packet->payload[command_start + 2] == 'S' || flow_packet->payload[command_start + 2] == 's')
		   && (flow_packet->payload[command_start + 3] == 'T' || flow_packet->payload[command_start + 3] == 't')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	} else if((flow_packet->payload[command_start] == 'N' || flow_packet->payload[command_start] == 'n')
		   && (flow_packet->payload[command_start + 1] == 'O' || flow_packet->payload[command_start + 1] == 'o')
		   && (flow_packet->payload[command_start + 2] == 'O' || flow_packet->payload[command_start + 2] == 'o')
		   && (flow_packet->payload[command_start + 3] == 'P' || flow_packet->payload[command_start + 3] == 'p')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	} else if((flow_packet->payload[command_start] == 'I' || flow_packet->payload[command_start] == 'i')
		   && (flow_packet->payload[command_start + 1] == 'D' || flow_packet->payload[command_start + 1] == 'd')
		   && (flow_packet->payload[command_start + 2] == 'L' || flow_packet->payload[command_start + 2] == 'l')
		   && (flow_packet->payload[command_start + 3] == 'E' || flow_packet->payload[command_start + 3] == 'e')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	}
      }
      if((command_start + 6) < flow_packet->payload_packet_len) {
	if((flow_packet->payload[command_start] == 'S' || flow_packet->payload[command_start] == 's')
	    && (flow_packet->payload[command_start + 1] == 'E' || flow_packet->payload[command_start + 1] == 'e')
	    && (flow_packet->payload[command_start + 2] == 'L' || flow_packet->payload[command_start + 2] == 'l')
	    && (flow_packet->payload[command_start + 3] == 'E' || flow_packet->payload[command_start + 3] == 'e')
	    && (flow_packet->payload[command_start + 4] == 'C' || flow_packet->payload[command_start + 4] == 'c')
	    && (flow_packet->payload[command_start + 5] == 'T' || flow_packet->payload[command_start + 5] == 't')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	} else if((flow_packet->payload[command_start] == 'E' || flow_packet->payload[command_start] == 'e')
		   && (flow_packet->payload[command_start + 1] == 'X' || flow_packet->payload[command_start + 1] == 'x')
		   && (flow_packet->payload[command_start + 2] == 'I' || flow_packet->payload[command_start + 2] == 'i')
		   && (flow_packet->payload[command_start + 3] == 'S' || flow_packet->payload[command_start + 3] == 's')
		   && (flow_packet->payload[command_start + 4] == 'T' || flow_packet->payload[command_start + 4] == 't')
		   && (flow_packet->payload[command_start + 5] == 'S' || flow_packet->payload[command_start + 5] == 's')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	} else if((flow_packet->payload[command_start] == 'A' || flow_packet->payload[command_start] == 'a')
		   && (flow_packet->payload[command_start + 1] == 'P' || flow_packet->payload[command_start + 1] == 'p')
		   && (flow_packet->payload[command_start + 2] == 'P' || flow_packet->payload[command_start + 2] == 'p')
		   && (flow_packet->payload[command_start + 3] == 'E' || flow_packet->payload[command_start + 3] == 'e')
		   && (flow_packet->payload[command_start + 4] == 'N' || flow_packet->payload[command_start + 4] == 'n')
		   && (flow_packet->payload[command_start + 5] == 'D' || flow_packet->payload[command_start + 5] == 'd')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	}
      }

    }

    if(saw_command == 1) {
      if((flow->l4.tcp.mail_imap_stage == 3)
	 || (flow->l4.tcp.mail_imap_stage == 5)
	 || (flow->l4.tcp.mail_imap_stage == 7)
	 ) {
	if((flow->protos.ftp_imap_pop_smtp.username[0] != '\0')
	   || (flow->l4.tcp.mail_imap_stage >= 7)) {
	  NDPI_LOG_INFO(ndpi_struct, "found MAIL_IMAP\n");
	  ndpi_int_mail_imap_add_connection(ndpi_struct, flow);
	}
	
	return;
      }
    }
  }

  if(flow_packet->payload_packet_len > 1 && flow_packet->payload[flow_packet->payload_packet_len - 1] == ' ') {
    NDPI_LOG_DBG2(ndpi_struct,
	     "maybe a split imap command -> need next packet and imap_stage is set to 4.\n");
    flow->l4.tcp.mail_imap_stage = 4;
    return;
  }

 imap_excluded:

  // skip over possible authentication hashes etc. that cannot be identified as imap commands or responses
  // if the packet count is low enough and at least one command or response was seen before
  if((flow_packet->payload_packet_len >= 2 && ntohs(get_u_int16_t(flow_packet->payload, flow_packet->payload_packet_len - 2)) == 0x0d0a)
      && flow->packet_counter < 6 && flow->l4.tcp.mail_imap_stage >= 1) {
    NDPI_LOG_DBG2(ndpi_struct,
	     "no imap command or response but packet count < 6 and imap stage >= 1 -> skip\n");
    return;
  }

  NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
}