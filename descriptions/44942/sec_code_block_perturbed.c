struct ndpi_iphdr iph;

  if(ipsize < 40)
    return(NULL);
  memset(&iph, 0, sizeof(iph));
  iph.version = IPVERSION;
  iph.saddr = ipv6_header->ip6_src.u6_addr.u6_addr32[2] + ipv6_header->ip6_src.u6_addr.u6_addr32[3];
  iph.daddr = ipv6_header->ip6_dst.u6_addr.u6_addr32[2] + ipv6_header->ip6_dst.u6_addr.u6_addr32[3];
  u_int8_t l4proto = ipv6_header->ip6_hdr.ip6_un1_nxt;
  u_int16_t ip_len = ntohs(ipv6_header->ip6_hdr.ip6_un1_plen);
  const u_int8_t *l4ptr = (((const u_int8_t *) ipv6_header) + sizeof(struct ndpi_ipv6hdr));
  if(ipsize < sizeof(struct ndpi_ipv6hdr) + ip_len)
    return(NULL);
  if(ndpi_handle_ipv6_extension_headers(ipsize - sizeof(struct ndpi_ipv6hdr), &l4ptr, &ip_len, &l4proto) != 0) {
    return(NULL);
  }