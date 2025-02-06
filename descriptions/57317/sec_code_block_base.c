struct ndpi_packet_struct *packet = &ndpi_struct->packet;
  u_int16_t num_found = 0;
  int32_t i;
  char buffer[64] = { '\0' }, rdnSeqBuf[2048];
  u_int rdn_len = 0;

  rdnSeqBuf[0] = '\0';