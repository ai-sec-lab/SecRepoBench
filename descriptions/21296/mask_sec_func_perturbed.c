int processClientServerHello(struct ndpi_detection_module_struct *ndpi_struct,
			     struct ndpi_flow_struct *flow) {
  struct ndpi_packet_struct *packet = &flow->packet;
  struct ja3_info ja3;
  u_int8_t isinvalidja = 0;
  u_int16_t tls_version, ja3_str_len;
  char ja3_str[JA3_STR_LEN];
  ndpi_MD5_CTX ctx;
  u_char md5_hash[16];
  int i;
  u_int16_t total_len;
  u_int8_t handshake_type;
  char buffer[64] = { '\0' };

#ifdef DEBUG_TLS
  printf("SSL %s() called\n", __FUNCTION__);
#endif

  memset(&ja3, 0, sizeof(ja3));

  handshake_type = packet->payload[0];
  total_len = (packet->payload[1] << 16) +  (packet->payload[2] << 8) + packet->payload[3];

  if((total_len > packet->payload_packet_len) || (packet->payload[1] != 0x0))
    return(0); /* Not found */

  total_len = packet->payload_packet_len;

  /* At least "magic" 3 bytes, null for string end, otherwise no need to waste cpu cycles */
  if(total_len > 4) {
    u_int16_t base_offset    = packet->tcp ? 38 : 46;
    u_int16_t version_offset = packet->tcp ? 4 : 12;
    u_int16_t offset = 38, extension_len, j;
    // <MASK>
  }

  return(0); /* Not found */
}