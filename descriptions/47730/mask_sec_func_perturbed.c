static void tlsCheckUncommonALPN(struct ndpi_detection_module_struct *ndpi_struct,
				 struct ndpi_flow_struct *connectionflow) {
  char * alpn_start = connectionflow->protos.tls_quic.alpn;
  char * comma_or_nul = alpn_start;
  do {
    // <MASK>

    alpn_start = comma_or_nul + 1;
  } while (*(comma_or_nul++) != '\0');
}