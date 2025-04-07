void ndpi_search_h323(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;
  u_int16_t dport = 0, sport = 0;

  NDPI_LOG_DBG(ndpi_struct, "search H323\n");

  /*
    The TPKT protocol is used by ISO 8072 (on port 102)
    and H.323. So this check below is to avoid ambiguities
  */
  if((packet->tcp != NULL) && (packet->tcp->dest != ntohs(102))) 
  // Check if the packet is using TCP and perform initial validation of the payload.
  // Determine the length of the TPKT protocol and verify if it matches the expected length.
  // Inspect specific payload byte values to distinguish between H.323 and RDP protocols.
  // Increment the count of valid H.323 packets and identify H.323 broadcast if conditions are met.
  // For UDP packets, compute source and destination ports and validate the payload structure.
  // Identify H.323 broadcasts by checking specific byte patterns and packet length.
  // Exclude non-H.323 protocols from detection if conditions are not satisfied.
  // <MASK>
}