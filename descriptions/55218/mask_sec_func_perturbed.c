static void ndpi_search_bittorrent(struct ndpi_detection_module_struct *detectionmodule,
				   struct ndpi_flow_struct *flow) {
  struct ndpi_packet_struct *packet = &detectionmodule->packet;
  char *bt_proto = NULL;

  NDPI_LOG_DBG(detectionmodule, "Search bittorrent\n");

  /* This is broadcast */
  if(packet->iph) {
    if((packet->iph->saddr == 0xFFFFFFFF) || (packet->iph->daddr == 0xFFFFFFFF))
      goto exclude_bt;

    if(packet->udp) {
      u_int16_t sport = ntohs(packet->udp->source), dport = ntohs(packet->udp->dest);

      if(is_port(sport, dport, 3544) /* teredo */
	 || is_port(sport, dport, 5246) || is_port(sport, dport, 5247) /* CAPWAP */) {
      exclude_bt:
	NDPI_EXCLUDE_PROTO(detectionmodule, flow);
	return;
      }
    }
  }

  if(flow->detected_protocol_stack[0] != NDPI_PROTOCOL_BITTORRENT) {
    if(packet->tcp != NULL) {
      ndpi_int_search_bittorrent_tcp(detectionmodule, flow);
    } else if(packet->udp != NULL) {
      /* UDP */
      const char *bt_search  = "BT-SEARCH * HTTP/1.1\r\n";
      const char *bt_search1 = "d1:ad2:id20:";

      if((ntohs(packet->udp->source) < 1024)
	 || (ntohs(packet->udp->dest) < 1024) /* High ports only */) {
	ndpi_skip_bittorrent(detectionmodule, flow, packet);
	return;
      }

      /*
	Check for uTP http://www.bittorrent.org/beps/bep_0029.html

	wireshark/epan/dissectors/packet-bt-utp.c
      */

	// <MASK>

      flow->bittorrent_stage++;

      if(flow->bittorrent_stage < 5) {
	/* We have detected bittorrent but we need to wait until we get a hash */

	if(packet->payload_packet_len > 19 /* min size */) {
	  if(ndpi_strnstr((const char *)packet->payload, ":target20:", packet->payload_packet_len)
	     || ndpi_strnstr((const char *)packet->payload, ":find_node1:", packet->payload_packet_len)
	     || ndpi_strnstr((const char *)packet->payload, "d1:ad2:id20:", packet->payload_packet_len)
	     || ndpi_strnstr((const char *)packet->payload, ":info_hash20:", packet->payload_packet_len)
	     || ndpi_strnstr((const char *)packet->payload, ":filter64", packet->payload_packet_len)
	     || ndpi_strnstr((const char *)packet->payload, "d1:rd2:id20:", packet->payload_packet_len)
	     || (bt_proto = ndpi_strnstr((const char *)packet->payload, BITTORRENT_PROTO_STRING, packet->payload_packet_len))
	     ) {
	  bittorrent_found:
	    if(bt_proto != NULL && ((u_int8_t *)&bt_proto[27] - packet->payload +
				    sizeof(flow->protos.bittorrent.hash)) < packet->payload_packet_len) {
	      memcpy(flow->protos.bittorrent.hash, &bt_proto[27], sizeof(flow->protos.bittorrent.hash));
	      flow->extra_packets_func = NULL; /* Nothing else to do */
	    }

	    NDPI_LOG_INFO(detectionmodule, "found BT: plain\n");
	    ndpi_add_connection_as_bittorrent(detectionmodule, flow, -1, 0, NDPI_CONFIDENCE_DPI);
	    return;
	  }
	}

	return;
      }

      ndpi_skip_bittorrent(detectionmodule, flow, packet);
    }
  }

  if(flow->packet_counter > 8) {
    ndpi_skip_bittorrent(detectionmodule, flow, packet);
  }
}