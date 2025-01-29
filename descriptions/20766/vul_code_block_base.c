#if LUCA
    if (header->caplen < ip_offset + ip_len + sizeof(struct ndpi_udphdr))
      return(nproto); /* Too short for UDP header*/
#endif
    struct ndpi_udphdr *udp = (struct ndpi_udphdr *)&packet[ip_offset+ip_len];