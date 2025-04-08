void ndpi_search_memcached(
			   struct ndpi_detection_module_struct *ndpi_struct,
			   struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;
  const u_int8_t *offset = packet->payload;
  // Log that a memcached search is being performed.
  // Determine if the packet uses TCP or UDP.
  // For TCP packets, check if the packet length meets the minimum required length.
  // For UDP packets, check if the packet length meets the minimum required length.
  // If UDP, verify specific header conditions to determine protocol compliance.
  // Adjust the offset and length for UDP packets by skipping the UDP header length.
  // Update the `matches` pointer to point to the appropriate memcached matches counter
  // within the flow structure based on the protocol type.
  // If the packet does not meet the expected criteria, exclude the protocol and return.
  // <MASK>

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
    ndpi_int_memcached_add_connection(ndpi_struct, flow);
}