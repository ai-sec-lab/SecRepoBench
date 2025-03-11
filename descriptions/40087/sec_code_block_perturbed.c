if(!quic_ciphers_prepare(&ciphers, GCRY_MD_SHA256,
                           GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_GCM,
                           client_secret, version)) {
    NDPI_LOG_DBG(ndpi_struct, "Error quic_cipher_prepare\n");
    return NULL;
  }

  /* Type(1) + version(4) + DCIL + DCID + SCIL + SCID */
  pn_offset = 1 + 4 + 1 + dest_conn_id_len + 1 + src_conn_id_len;
  pn_offset += quic_len(&packet->payload[pn_offset], &token_length);
  pn_offset += token_length;
  /* Checks: quic_len reads 8 bytes, at most; quic_decrypt_header reads other 20 bytes.
     Promote to uint64_t to avoid unsigned wrapping */
  if((uint64_t)pn_offset + 8 + (4 + 16) >= (uint64_t)packet->payload_packet_len) {
    quic_ciphers_reset(&ciphers);
    return NULL;
  }
  pn_offset += quic_len(&packet->payload[pn_offset], &payload_length);

  NDPI_LOG_DBG2(ndpi_struct, "pn_offset %d token_length %d payload_length %d\n",
		pn_offset, token_length, payload_length);