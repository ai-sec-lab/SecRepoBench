int alpn_len;

    comma_or_nul = strchr(comma_or_nul, ',');

    if(comma_or_nul == NULL)
      comma_or_nul = alpn_start + strlen(alpn_start);

    alpn_len = comma_or_nul - alpn_start;

    if(!is_a_common_alpn(ndpi_struct, alpn_start, alpn_len)) {
      char str[64] = { '\0' };
      
#ifdef DEBUG_TLS
      printf("TLS uncommon ALPN found: %.*s\n", (int)alpn_len, alpn_start);
#endif

      strncpy(str, alpn_start, alpn_len);
      ndpi_set_risk(ndpi_struct, connectionflow, NDPI_TLS_UNCOMMON_ALPN, str);
      break;
    }