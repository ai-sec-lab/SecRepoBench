if (memcmp(&packet->line[i].ptr[j], "SEND ", 5) == 0
		  || (memcmp(&packet->line[i].ptr[j], "CHAT", 4) == 0)
		  || (memcmp(&packet->line[i].ptr[j], "chat", 4) == 0)
		  || (j+7 < packet->line[i].len && memcmp(&packet->line[i].ptr[j], "sslchat", 7) == 0)
		  || (memcmp(&packet->line[i].ptr[j], "TSEND", 5) == 0)) {
		NDPI_LOG_DBG2(ndpi_struct, "found CHAT,chat,sslchat,TSEND.");
		j += 4;

		while (packet->line[i].len > j &&
		       ((packet->line[i].ptr[j] >= 'a' && packet->line[i].ptr[j] <= 'z')
			|| (packet->line[i].ptr[j] >= 'A' && packet->line[i].ptr[j] <= 'Z')
			|| (packet->line[i].ptr[j] >= '0' && packet->line[i].ptr[j] <= '9')
			|| (packet->line[i].ptr[j] >= ' ')
			|| (packet->line[i].ptr[j] >= '.')
			|| (packet->line[i].ptr[j] >= '-'))) {

		  if (packet->line[i].ptr[j] == ' ') {
		    space++;
		    NDPI_LOG_DBG2(ndpi_struct, "space %u.", space);
		  }
		  if (space == 3) {
		    j++;
		    NDPI_LOG_DBG2(ndpi_struct, "read port.");
		    if (src_id != NULL) {
		      k = j;
		      port =
			ntohs_ndpi_bytestream_to_number
			(&packet->line[i].ptr[j], packet->payload_packet_len - j, &j);
		      NDPI_LOG_DBG2(ndpi_struct, "port %u.",
			       port);
		      j = k;
		      // hier jetzt überlegen, wie die ports abgespeichert werden sollen
		      if (src_id->irc_number_of_port < NDPI_PROTOCOL_IRC_MAXPORT)
			NDPI_LOG_DBG2(ndpi_struct, "src->irc_number_of_port < NDPI_PROTOCOL_IRC_MAXPORT.");
		      if (src_id->irc_number_of_port < NDPI_PROTOCOL_IRC_MAXPORT && port != 0) {
			if (!ndpi_is_duplicate(src_id, port)) {
			  src_id->irc_port[src_id->irc_number_of_port]
			    = port;
			  src_id->irc_number_of_port++;
			  NDPI_LOG_DBG2(ndpi_struct, "found port=%d jjeeeeeeeeeeeeeeeeeeeeeeeee",
			     ntohs(get_u_int16_t(src_id->irc_port, 0)));
			}
			src_id->irc_ts = packet->tick_timestamp;
		      } else if (port != 0 && src_id->irc_number_of_port == NDPI_PROTOCOL_IRC_MAXPORT) {
			if (!ndpi_is_duplicate(src_id, port)) {
			  less = 0;
			  NDPI_IRC_FIND_LESS(src_id->last_time_port_used, less);
			  src_id->irc_port[less] = port;
			  NDPI_LOG_DBG2(ndpi_struct, "found port=%d", ntohs(get_u_int16_t(src_id->irc_port, 0)));
			}
			src_id->irc_ts = packet->tick_timestamp;
		      }
		      if (dst == NULL) {
			break;
		      }
		    }
		    if (dst != NULL) {
		      port = ntohs_ndpi_bytestream_to_number
			(&packet->line[i].ptr[j], packet->payload_packet_len - j, &j);
		      NDPI_LOG_DBG2(ndpi_struct, "port %u.", port);
		      // hier das gleiche wie oben.
		      /* hier werden NDPI_PROTOCOL_IRC_MAXPORT ports pro irc flows mitgespeichert. könnte man denn nicht ein-
		       * fach an die dst oder src einen flag setzten, dass dieser port für eine bestimmte
		       * zeit ein irc-port bleibt?
		       */
		      if (dst->irc_number_of_port < NDPI_PROTOCOL_IRC_MAXPORT && port != 0) {
			if (!ndpi_is_duplicate(dst, port)) {
			  dst->irc_port[dst->irc_number_of_port]
			    = port;
			  dst->irc_number_of_port++;
			  NDPI_LOG_DBG2(ndpi_struct, "found port=%d", ntohs(get_u_int16_t(dst->irc_port, 0)));
			  NDPI_LOG_DBG2(ndpi_struct, "juuuuuuuuuuuuuuuu");
			}
			dst->irc_ts = packet->tick_timestamp;
		      } else if (port != 0 && dst->irc_number_of_port == NDPI_PROTOCOL_IRC_MAXPORT) {
			if (!ndpi_is_duplicate(dst, port)) {
			  less = 0;
			  NDPI_IRC_FIND_LESS(dst->last_time_port_used, less);
			  dst->irc_port[less] = port;

			  NDPI_LOG_DBG2(ndpi_struct, "found port=%d", ntohs(get_u_int16_t(dst->irc_port, 0)));
			}
			dst->irc_ts = packet->tick_timestamp;
		      }

		      break;
		    }
		  }


		  j++;
		}

	      }