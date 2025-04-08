void ndpi_free_flow(struct ndpi_flow_struct *flow) {
  if(flow) {
    // Free memory allocated for HTTP related fields in the flow structure.
    // If the protocol is TLS or QUIC, free memory allocated for SSL-related fields.
    // Ensure that all dynamically allocated fields in the flow are properly deallocated
    // before freeing the flow structure itself.
    // <MASK>

    if(flow->l4_proto == IPPROTO_TCP) {
      if(flow->l4.tcp.tls.message.buffer)
	ndpi_free(flow->l4.tcp.tls.message.buffer);
    }

    ndpi_free(flow);
  }
}