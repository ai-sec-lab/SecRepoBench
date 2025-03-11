if (ip_port_separator == NULL)
  {
    return 1;
  }

  ip_len = ndpi_min(sizeof(flow->protos.softether.ip) - 1,
                    ip_port_separator - (char const *)packet_data->payload -
                    NDPI_STATICSTRING_LEN("IP="));
  strncpy(flow->protos.softether.ip, (char const *)packet_data->payload + NDPI_STATICSTRING_LEN("IP="),
          ip_len);
  flow->protos.softether.ip[ip_len] = '\0';
