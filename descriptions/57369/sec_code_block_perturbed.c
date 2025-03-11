if(header->caplen < iphdroffset + ip_len + sizeof(struct ndpi_udphdr) + sizeof(struct ndpi_vxlanhdr)) {
    return false;
  }
  u_int32_t vxlan_dst_port  = ntohs(4789);
  struct ndpi_udphdr *udp = (struct ndpi_udphdr *)&packet[iphdroffset+ip_len];
  u_int offset = iphdroffset + ip_len + sizeof(struct ndpi_udphdr);
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