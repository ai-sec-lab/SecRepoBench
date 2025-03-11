static bool ndpi_is_valid_vxlan(const struct pcap_pkthdr *header, const u_char *packet, u_int16_t iphdroffset, u_int16_t ip_len){
  // <MASK>
  if((udp->dest == vxlan_dst_port || udp->source == vxlan_dst_port) &&
    (packet[offset] == 0x8) &&
    (packet[offset + 1] == 0x0) &&
    (packet[offset + 2] == 0x0) &&
    (packet[offset + 3] == 0x0) &&
    (packet[offset + 7] ==  0x0)) {
    return true;
    }
  return false;
}