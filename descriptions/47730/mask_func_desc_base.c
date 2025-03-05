static void tlsCheckUncommonALPN(struct ndpi_detection_module_struct *ndpi_struct,
				 struct ndpi_flow_struct *flow) {
  char * alpn_start = flow->protos.tls_quic.alpn;
  char * comma_or_nul = alpn_start;
  do {
    // Iterate through the ALPN string, delimited by commas, to extract each ALPN identifier.
    // For each ALPN identifier, check if it is not common using a helper function.
    // If an uncommon ALPN is found, prepare a string representation of the ALPN,
    // log a debug message if debugging is enabled, and set a risk using the detection module.
    // <MASK>

    alpn_start = comma_or_nul + 1;
  } while (*(comma_or_nul++) != '\0');
}