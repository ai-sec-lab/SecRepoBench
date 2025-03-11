static uint8_t *decrypt_initial_packet(struct ndpi_detection_module_struct *ndpi_struct,
				       struct ndpi_flow_struct *flow,
				       const uint8_t *dest_conn_id, uint8_t dest_conn_id_len,
				       uint8_t src_conn_id_len, uint32_t version,
				       uint32_t *clear_payload_len)
{
  uint64_t token_length, payload_length, packet_number;
  struct ndpi_packet_struct *packet = &ndpi_struct->packet;
  uint8_t first_byte;
  uint32_t pkn32, pn_offset, pkn_len, offset;
  quic_ciphers ciphers; /* Client initial ciphers */
  quic_decrypt_result_t decryption = { 0, 0};
  uint8_t client_secret[HASH_SHA2_256_LENGTH];

  memset(&ciphers, '\0', sizeof(ciphers));
  if(quic_derive_initial_secrets(version, dest_conn_id, dest_conn_id_len,
				 client_secret) != 0) {
    NDPI_LOG_DBG(ndpi_struct, "Error quic_derive_initial_secrets\n");
    return NULL;
  }

  /* Packet numbers are protected with AES128-CTR,
     Initial packets are protected with AEAD_AES_128_GCM. */
  // <MASK>

  if (pn_offset + payload_length > packet->payload_packet_len) {
    NDPI_LOG_DBG(ndpi_struct, "Too short %d %d\n", pn_offset + payload_length,
                 packet->payload_packet_len);
    quic_ciphers_reset(&ciphers);
    return NULL;
  }

  if(!quic_decrypt_header(&packet->payload[0], pn_offset, &ciphers.hp_cipher,
			  GCRY_CIPHER_AES128, &first_byte, &pkn32, 0)) {
    quic_ciphers_reset(&ciphers);
    return NULL;
  }
  NDPI_LOG_DBG2(ndpi_struct, "first_byte 0x%x pkn32 0x%x\n", first_byte, pkn32);

  pkn_len = (first_byte & 3) + 1;
  /* TODO: is it always true in Initial Packets? */
  packet_number = pkn32;

  offset = pn_offset + pkn_len;
  if (!(pn_offset + payload_length >= offset + 16)) {
    NDPI_LOG_DBG(ndpi_struct, "No room for Auth Tag %d %d",
                 pn_offset + payload_length, offset);
    quic_ciphers_reset(&ciphers);
    return NULL;
  }
  quic_decrypt_message(&ciphers.pp_cipher, &packet->payload[0], pn_offset + payload_length,
		       offset, first_byte, pkn_len, packet_number, &decryption);

  quic_ciphers_reset(&ciphers);

  if(decryption.data_len) {
    *clear_payload_len = decryption.data_len;
    return decryption.data;
  }
  return NULL;
}