static void process_chlo(struct ndpi_detection_module_struct *detectionmodule,
			 struct ndpi_flow_struct *flow,
			 const u_int8_t *crypto_data, uint32_t crypto_data_len)
{
  const uint8_t *tag;
  uint32_t i;
  uint16_t num_tags;
  uint32_t prev_offset;
  uint32_t tag_offset_start, offset, len, sni_len;
  ndpi_protocol_match_result ret_match;
  int sni_found = 0, ua_found = 0;

  if(crypto_data_len < 6)
    return;
  if(memcmp(crypto_data, "CHLO", 4) != 0) {
#ifdef QUIC_DEBUG
    NDPI_LOG_ERR(detectionmodule, "Unexpected handshake message");
#endif
    return;
  }
  num_tags = le16toh(*(uint16_t *)&crypto_data[4]);

  tag_offset_start = 8 + 8 * num_tags;
  prev_offset = 0;
  for(i = 0; i < num_tags; i++) {
    if(8 + 8 * i + 8 >= crypto_data_len)
      break;
    tag = &crypto_data[8 + 8 * i];
    offset = le32toh(*((u_int32_t *)&crypto_data[8 + 8 * i + 4]));
    if(prev_offset > offset)
      break;
    len = offset - prev_offset;
    /* Promote to uint64_t to avoid unsigned wrapping */
    if((uint64_t)tag_offset_start + prev_offset + len > (uint64_t)crypto_data_len)
      break;
#if 0
    printf("crypto_data_len %u tag_offset_start %u prev_offset %u offset %u len %u\n",
	   crypto_data_len, tag_offset_start, prev_offset, offset, len);
#endif
    if(memcmp(tag, "SNI\0", 4) == 0) {
      sni_len = MIN(len, sizeof(flow->protos.tls_quic_stun.tls_quic.client_requested_server_name) - 1);
      memcpy(flow->protos.tls_quic_stun.tls_quic.client_requested_server_name,
             &crypto_data[tag_offset_start + prev_offset], sni_len);
      flow->protos.tls_quic_stun.tls_quic.client_requested_server_name[sni_len] = '\0';

      NDPI_LOG_DBG2(detectionmodule, "SNI: [%s]\n",
                    flow->protos.tls_quic_stun.tls_quic.client_requested_server_name);

      ndpi_match_host_subprotocol(detectionmodule, flow,
                                  (char *)flow->protos.tls_quic_stun.tls_quic.client_requested_server_name,
                                  strlen((const char*)flow->protos.tls_quic_stun.tls_quic.client_requested_server_name),
                                  &ret_match, NDPI_PROTOCOL_QUIC);
      flow->protos.tls_quic_stun.tls_quic.hello_processed = 1; /* Allow matching of custom categories */

      ndpi_check_dga_name(detectionmodule, flow,
                          flow->protos.tls_quic_stun.tls_quic.client_requested_server_name, 1);

      sni_found = 1;
      if (ua_found)
        return;
    }

    if(memcmp(tag, "UAID", 4) == 0) {
      u_int uaid_offset = tag_offset_start + prev_offset;
            
      NDPI_LOG_DBG2(detectionmodule, "UA: [%.*s]\n", len, &crypto_data[uaid_offset]);
	
      http_process_user_agent(detectionmodule, flow, &crypto_data[uaid_offset], len); /* http.c */
      ua_found = 1;
	
      if (sni_found)
        return;
    }

    prev_offset = offset;
  }
  if(i != num_tags)
    NDPI_LOG_DBG(detectionmodule, "Something went wrong in tags iteration\n");

  /* Add check for missing SNI */
  if(flow->protos.tls_quic_stun.tls_quic.client_requested_server_name[0] == '\0') {
    /* This is a bit suspicious */
    ndpi_set_risk(detectionmodule, flow, NDPI_TLS_MISSING_SNI);
  }
}