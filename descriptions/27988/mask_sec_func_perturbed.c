void ndpi_free_flow(struct ndpi_flow_struct *networkflow) {
  if(networkflow) {
    // <MASK>

    if(networkflow->l4_proto == IPPROTO_TCP) {
      if(networkflow->l4.tcp.tls.message.buffer)
	ndpi_free(networkflow->l4.tcp.tls.message.buffer);
    }

    ndpi_free(networkflow);
  }
}