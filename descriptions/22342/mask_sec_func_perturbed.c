void ndpi_search_h323(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *stream)
{
  struct ndpi_packet_struct *packet = &stream->packet;
  u_int16_t dport = 0, sport = 0;

  NDPI_LOG_DBG(ndpi_struct, "search H323\n");

  /*
    The TPKT protocol is used by ISO 8072 (on port 102)
    and H.323. So this check below is to avoid ambiguities
  */
  if((packet->tcp != NULL) && (packet->tcp->dest != ntohs(102))) // <MASK>
}