static void tlsCheckUncommonALPN(struct ndpi_detection_module_struct *ndpi_struct,
				 struct ndpi_flow_struct *connectionflow) {
  char * alpn_start = connectionflow->protos.tls_quic.alpn;
  char * comma_or_nul = alpn_start;
  do {
    size_t alpn_len;

    comma_or_nul = strchr(comma_or_nul, ',');

    if(comma_or_nul == NULL)
      comma_or_nul = alpn_start + strlen(alpn_start);

    alpn_len = comma_or_nul - alpn_start;

    if(!is_a_common_alpn(ndpi_struct, alpn_start, alpn_len)) {
      char str[64];
      size_t str_len;
      
#ifdef DEBUG_TLS
      printf("TLS uncommon ALPN found: %.*s\n", (int)alpn_len, alpn_start);
#endif

      str[0] = '\0';
      str_len = ndpi_min(alpn_len, sizeof(str));
      if(str_len > 0) {
        strncpy(str, alpn_start, str_len);
        str[str_len - 1] = '\0';
      }

      ndpi_set_risk(ndpi_struct, connectionflow, NDPI_TLS_UNCOMMON_ALPN, str);
      break;
    }

    alpn_start = comma_or_nul + 1;
  } while (*(comma_or_nul++) != '\0');
}