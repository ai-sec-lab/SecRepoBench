int processClientServerHello(struct ndpi_detection_module_struct *ndpi_struct,
			     struct ndpi_flow_struct *flow) {
  struct ndpi_packet_struct *packet = &flow->packet;
  struct ja3_info ja3;
  u_int8_t invalid_ja3 = 0;
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
    u_int8_t  session_id_len =  packet->tcp ? packet->payload[offset] : packet->payload[46];

#ifdef DEBUG_TLS
    printf("SSL [len: %u][handshake_type: %02X]\n", packet->payload_packet_len, handshake_type);
#endif

    tls_version = ntohs(*((u_int16_t*)&packet->payload[version_offset]));
    flow->protos.stun_ssl.ssl.ssl_version = ja3.tls_handshake_version = tls_version;

    if(handshake_type == 0x02 /* Server Hello */) 
    // Parse the TLS handshake message from the packet to determine whether it is a Client Hello or Server Hello.
    // Depending on the type of handshake message, extract relevant information such as cipher suites and TLS extensions.
    // For a Server Hello, adjust offsets based on the session ID length and extract the server's cipher suite.
    // For a Client Hello, parse the cipher suites and extensions, skipping GREASE values, and extract supported versions and other extension information.
    // Construct a JA3 string based on extracted details, including TLS version, cipher suites, and extensions for fingerprinting purposes.
    // Compute an MD5 hash of the JA3 string for both Client and Server handshakes to create a unique JA3 fingerprint.
    // Store the computed JA3 hash in the flow structure for later analysis or comparison.
    // Return a specific code to indicate the type of handshake processed, or zero if parsing was unsuccessful.
    // <MASK>
  }

  return(0); /* Not found */
}