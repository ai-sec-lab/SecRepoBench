u_int16_t length = packet->payload_packet_len;
  u_int8_t *matches;

  NDPI_LOG_DBG(ndpi_struct, "search memcached\n");

  if (packet->tcp != NULL) {
    if (packet->payload_packet_len < MEMCACHED_MIN_LEN) {
      NDPI_EXCLUDE_PROTO(ndpi_struct, stream);
      return;
    }

    matches = &stream->l4.tcp.memcached_matches;
  }
  else if (packet->udp != NULL) {
    if (packet->payload_packet_len < MEMCACHED_MIN_UDP_LEN) {
      NDPI_EXCLUDE_PROTO(ndpi_struct, stream);
      return;
    }

    if ((offset[4] == 0x00 && offset[5] == 0x00) ||
	offset[6] != 0x00 || offset[7] != 0x00) {
      NDPI_EXCLUDE_PROTO(ndpi_struct, stream);
      return;
    }

    offset += MEMCACHED_UDP_HDR_LEN;
    length -= MEMCACHED_UDP_HDR_LEN;
    matches = &stream->l4.udp.memcached_matches;
  }
  else {
    NDPI_EXCLUDE_PROTO(ndpi_struct, stream);
    return;
  }