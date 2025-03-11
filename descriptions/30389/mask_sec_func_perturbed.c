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
    // <MASK>

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