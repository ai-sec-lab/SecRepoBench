static struct ndpi_flow_info *get_ndpi_flow_info6(struct ndpi_workflow * workflow,
						  u_int16_t vlan_id,
						  ndpi_packet_tunnel tunnel_type,
						  const struct ndpi_ipv6hdr *iph6,
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
  // Create an IPv4 header structure and initialize it to zero.
  // Extract source and destination addresses from the IPv6 header
  // and assign them to the corresponding fields in the IPv4 header structure.
  // Retrieve the protocol type and payload length from the IPv6 header.
  // Determine the pointer to the layer 4 protocol data by offsetting
  // from the start of the IPv6 header.
  // Handle IPv6 extension headers, adjusting the protocol data pointer,
  // and update the protocol and payload length accordingly.
  // <MASK>
  iph.protocol = l4proto;

  return(get_ndpi_flow_info(workflow, 6, vlan_id, tunnel_type,
			    &iph, iph6, ip_offset, ipsize,
			    ip_len, l4ptr - (const u_int8_t *)iph6,
			    tcph, udph, sport, dport,
			    proto, payload,
			    payload_len, src_to_dst_direction, when));
}