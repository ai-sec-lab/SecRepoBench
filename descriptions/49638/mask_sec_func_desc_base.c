static int dissect_softether_ip_port(struct ndpi_flow_struct *flow,
                                     struct ndpi_packet_struct const *packet)
{
  char * ip_port_separator;
  size_t ip_len, port_len;

  if (packet->payload_packet_len < NDPI_STATICSTRING_LEN("IP=") +
                                   NDPI_STATICSTRING_LEN(",PORT="))
  {
    return 1;
  }

  if (strncmp((char *)&packet->payload[0], "IP=", NDPI_STATICSTRING_LEN("IP=")) != 0)
  {
    return 1;
  }

  ip_port_separator = ndpi_strnstr((char const *)packet->payload + NDPI_STATICSTRING_LEN("IP="),
                                   ",PORT=",
                                   packet->payload_packet_len - NDPI_STATICSTRING_LEN("IP="));
  // Check if the IP and PORT separator was found in the payload.
  // If found, extract the IP address string from the packet's payload.
  // Copy the extracted IP address into the flow structure, ensuring it fits within the allocated space.
  // Null-terminate the copied IP address in the flow structure.
  // <MASK>

  port_len = ndpi_min(sizeof(flow->protos.softether.port) - 1,
                      ip_port_separator - (char const *)packet->payload -
                      NDPI_STATICSTRING_LEN("IP=") - NDPI_STATICSTRING_LEN(",PORT="));
  strncpy(flow->protos.softether.port, ip_port_separator + NDPI_STATICSTRING_LEN(",PORT="),
          port_len);
  flow->protos.softether.port[port_len] = '\0';

  return 0;
}