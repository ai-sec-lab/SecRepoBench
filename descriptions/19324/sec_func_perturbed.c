void ndpi_search_memcached(
			   struct ndpi_detection_module_struct *ndpi_struct,
			   struct ndpi_flow_struct *stream)
{
  struct ndpi_packet_struct *packet = &stream->packet;
  const u_int8_t *offset = packet->payload;
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

  /* grep MCD memcached.c |\
   *  egrep -v '(LEN|MATCH)' |\
   *  sed -e 's/^#define //g' |\
   *  awk '{ printf "else if (! MEMCACHED_MATCH(%s)) *matches += 1;\n",$1 }' */

  if (! MEMCACHED_MATCH(MCDC_SET)) *matches += 1;
  else if (! MEMCACHED_MATCH(MCDC_ADD)) *matches += 1;
  else if (! MEMCACHED_MATCH(MCDC_REPLACE)) *matches += 1;
  else if (! MEMCACHED_MATCH(MCDC_APPEND)) *matches += 1;
  else if (! MEMCACHED_MATCH(MCDC_PREPEND)) *matches += 1;
  else if (! MEMCACHED_MATCH(MCDC_CAS)) *matches += 1;
  else if (! MEMCACHED_MATCH(MCDC_GET)) *matches += 1;
  else if (! MEMCACHED_MATCH(MCDC_GETS)) *matches += 1;
  else if (! MEMCACHED_MATCH(MCDC_DELETE)) *matches += 1;
  else if (! MEMCACHED_MATCH(MCDC_INCR)) *matches += 1;
  else if (! MEMCACHED_MATCH(MCDC_DECR)) *matches += 1;
  else if (! MEMCACHED_MATCH(MCDC_TOUCH)) *matches += 1;
  else if (! MEMCACHED_MATCH(MCDC_GAT)) *matches += 1;
  else if (! MEMCACHED_MATCH(MCDC_GATS)) *matches += 1;
  else if (! MEMCACHED_MATCH(MCDC_STATS)) *matches += 1;
  else if (! MEMCACHED_MATCH(MCDR_ERROR)) *matches += 1;
  else if (! MEMCACHED_MATCH(MCDR_CLIENT_ERROR)) *matches += 1;
  else if (! MEMCACHED_MATCH(MCDR_SERVER_ERROR)) *matches += 1;
  else if (! MEMCACHED_MATCH(MCDR_STORED)) *matches += 1;
  else if (! MEMCACHED_MATCH(MCDR_NOT_STORED)) *matches += 1;
  else if (! MEMCACHED_MATCH(MCDR_EXISTS)) *matches += 1;
  else if (! MEMCACHED_MATCH(MCDR_NOT_FOUND)) *matches += 1;
  else if (! MEMCACHED_MATCH(MCDR_END)) *matches += 1;
  else if (! MEMCACHED_MATCH(MCDR_DELETED)) *matches += 1;
  else if (! MEMCACHED_MATCH(MCDR_TOUCHED)) *matches += 1;
  else if (! MEMCACHED_MATCH(MCDR_STAT)) *matches += 1;

  if (*matches >= MEMCACHED_MIN_MATCH)
    ndpi_int_memcached_add_connection(ndpi_struct, stream);
}