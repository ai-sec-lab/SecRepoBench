if(
	   (packet->payload_packet_len > 22 && strncmp((const char*)packet->payload, bt_search, strlen(bt_search)) == 0) ||
	   (packet->payload_packet_len > 12 && strncmp((const char*)packet->payload, bt_search1, strlen(bt_search1)) == 0)
	   ) {
	  ndpi_add_connection_as_bittorrent(detectionmodule, flow, -1, 1, NDPI_CONFIDENCE_DPI);
	  return;
	} else if(packet->payload_packet_len >= 20) {
	  /* Check if this is protocol v0 */
	  u_int8_t v0_extension = packet->payload[17];
	  u_int8_t v0_flags     = packet->payload[18];

	  if(is_utpv1_pkt(packet->payload, packet->payload_packet_len)) {
	    bt_proto = ndpi_strnstr((const char *)&packet->payload[20], BITTORRENT_PROTO_STRING, packet->payload_packet_len-20);
	    goto bittorrent_found;
	  } else if((packet->payload[0]== 0x60)
		    && (packet->payload[1]== 0x0)
		    && (packet->payload[2]== 0x0)
		    && (packet->payload[3]== 0x0)
		    && (packet->payload[4]== 0x0)) {
	    /* Heuristic */
	    bt_proto = ndpi_strnstr((const char *)&packet->payload[20], BITTORRENT_PROTO_STRING, packet->payload_packet_len-20);
	    goto bittorrent_found;
	    /* CSGO/DOTA conflict */
	  } else if((v0_flags < 6 /* ST_NUM_STATES */) && (v0_extension < 3 /* EXT_NUM_EXT */)) {
	    u_int32_t ts = ntohl(*((u_int32_t*)&(packet->payload[4])));
	    u_int32_t now;

	    now = (u_int32_t)(packet->current_time_ms / 1000);

	    if((ts < (now+86400)) && (ts > (now-86400))) {
	      bt_proto = ndpi_strnstr((const char *)&packet->payload[20], BITTORRENT_PROTO_STRING, packet->payload_packet_len-20);
	      goto bittorrent_found;
	    }
	  } else if(ndpi_strnstr((const char *)&packet->payload[20], BITTORRENT_PROTO_STRING, packet->payload_packet_len-20)
		    ) {
	    goto bittorrent_found;
	  }

	}