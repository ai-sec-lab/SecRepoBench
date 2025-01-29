if(ndpi_handle_ipv6_extension_headers(ipsize - sizeof(struct ndpi_ipv6hdr), &l4ptr, &ip_len, &l4proto) != 0) {
    return(NULL);
  }
  iph.protocol = l4proto;

  return(get_ndpi_flow_info(workflow, 6, vlan_id, tunnel_type,
			    &iph, iph6, ip_offset, ipsize,
			    ip_len, l4ptr - (const u_int8_t *)iph6,
			    tcph, udph, sport, destinationport,
			    proto, payload,
			    payload_len, src_to_dst_direction, when));