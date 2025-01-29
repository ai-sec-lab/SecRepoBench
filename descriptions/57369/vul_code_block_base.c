if(header->caplen < ip_offset + ip_len + sizeof(struct ndpi_vxlanhdr)) {
    return false;
  }
  u_int32_t vxlan_dst_port  = ntohs(4789);
  struct ndpi_udphdr *udp = (struct ndpi_udphdr *)&packet[ip_offset+ip_len];
  u_int offset = ip_offset + ip_len + sizeof(struct ndpi_udphdr);
  /**
   * rfc-7348 
   *    VXLAN Header:  This is an 8-byte field that has:

    - Flags (8 bits): where the I flag MUST be set to 1 for a valid
      VXLAN Network ID (VNI).  The other 7 bits (designated "R") are
      reserved fields and MUST be set to zero on transmission and
      ignored on receipt.

    - VXLAN Segment ID/VXLAN Network Identifier (VNI): this is a
      24-bit value used to designate the individual VXLAN overlay
      network on which the communicating VMs are situated.  VMs in
      different VXLAN overlay networks cannot communicate with each
      other.

    - Reserved fields (24 bits and 8 bits): MUST be set to zero on
      transmission and ignored on receipt.
         VXLAN Header:
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |R|R|R|R|I|R|R|R|            Reserved                           |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                VXLAN Network Identifier (VNI) |   Reserved    |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  */
  if((udp->dest == vxlan_dst_port || udp->source == vxlan_dst_port) &&
    (packet[offset] == 0x8) &&
    (packet[offset + 1] == 0x0) &&
    (packet[offset + 2] == 0x0) &&
    (packet[offset + 3] == 0x0) &&
    (packet[offset + 7] ==  0x0)) {
    return true;
    }
  return false;