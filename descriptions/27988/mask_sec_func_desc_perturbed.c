void ndpi_free_flow(struct ndpi_flow_struct *networkflow) {
  if(networkflow) {
    // Free memory allocated for HTTP related fields in the flow structure.
    // If the protocol is TLS or QUIC, free memory allocated for SSL-related fields.
    // Ensure that all dynamically allocated fields in the flow are properly deallocated
    // before freeing the flow structure itself.
    // <MASK>

    if(networkflow->l4_proto == IPPROTO_TCP) {
      if(networkflow->l4.tcp.tls.message.buffer)
	ndpi_free(networkflow->l4.tcp.tls.message.buffer);
    }

    ndpi_free(networkflow);
  }
}