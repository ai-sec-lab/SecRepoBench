u_int16_t spotify_port = htons(57621);

    if((packet->udp->source == spotify_port)
       && (packet->udp->dest == spotify_port)) {
      if(payload_len > 2) {
	if(memcmp(packet->payload, "SpotUdp", 7) == 0) {
	  NDPI_LOG_INFO(ndpi_struct, "found spotify udp dissector\n");
	  ndpi_int_spotify_add_connection(ndpi_struct, flow, 0);
	  return;
	}
      }
    }