static int dissect_softether_ip_port(struct ndpi_flow_struct *flow,
                                     struct ndpi_packet_struct const *packet_data)
{
  char * ip_port_separator;
  size_t ip_len, port_len;

  if (packet_data->payload_packet_len < NDPI_STATICSTRING_LEN("IP=") +
                                   NDPI_STATICSTRING_LEN(",PORT="))
  {
    return 1;
  }

  if (strncmp((char *)&packet_data->payload[0], "IP=", NDPI_STATICSTRING_LEN("IP=")) != 0)
  {
    return 1;
  }

  ip_port_separator = ndpi_strnstr((char const *)packet_data->payload + NDPI_STATICSTRING_LEN("IP="),
                                   ",PORT=",
                                   packet_data->payload_packet_len - NDPI_STATICSTRING_LEN("IP="));
  if (ip_port_separator == NULL)
  {
    return 1;
  }

  if (ip_port_separator < (char const *)packet_data->payload + NDPI_STATICSTRING_LEN("IP="))
  {
    return 1;
  }

  ip_len = ndpi_min(sizeof(flow->protos.softether.ip) - 1,
                    ip_port_separator - (char const *)packet_data->payload -
                    NDPI_STATICSTRING_LEN("IP="));
  strncpy(flow->protos.softether.ip, (char const *)packet_data->payload + NDPI_STATICSTRING_LEN("IP="),
          ip_len);
  flow->protos.softether.ip[ip_len] = '\0';

  if (ip_port_separator < (char const *)packet_data->payload +
                          NDPI_STATICSTRING_LEN("IP=") + NDPI_STATICSTRING_LEN(",PORT="))
  {
    return 1;
  }

  port_len = ndpi_min(sizeof(flow->protos.softether.port) - 1,
                      ip_port_separator - (char const *)packet_data->payload -
                      NDPI_STATICSTRING_LEN("IP=") - NDPI_STATICSTRING_LEN(",PORT="));
  strncpy(flow->protos.softether.port, ip_port_separator + NDPI_STATICSTRING_LEN(",PORT="),
          port_len);
  flow->protos.softether.port[port_len] = '\0';

  return 0;
}