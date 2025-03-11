static struct ndpi_flow_info *get_ndpi_flow_info6(struct ndpi_workflow * workflow,
						  u_int16_t vlan_id,
						  ndpi_packet_tunnel tunnel_type,
						  const struct ndpi_ipv6hdr *ipv6_header,
						  u_int16_t ip_offset,
						  u_int16_t ipsize,
						  struct ndpi_tcphdr **tcph,
						  struct ndpi_udphdr **udph,
						  u_int16_t *sport, u_int16_t *dport,
						  u_int8_t *proto,
						  u_int8_t **payload,
						  u_int16_t *payload_len,
						  u_int8_t *src_to_dst_direction,
                                                  pkt_timeval when) {
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
  iph.protocol = l4proto;

  return(get_ndpi_flow_info(workflow, 6, vlan_id, tunnel_type,
			    &iph, ipv6_header, ip_offset, ipsize,
			    ip_len, l4ptr - (const u_int8_t *)ipv6_header,
			    tcph, udph, sport, dport,
			    proto, payload,
			    payload_len, src_to_dst_direction, when));
}