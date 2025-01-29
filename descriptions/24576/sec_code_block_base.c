if(packet[0] == 0x0f || packet[0] == 0x8f) {
      chdlc = (struct ndpi_chdlc *) &packet[eth_offset];
      ip_offset = sizeof(struct ndpi_chdlc); /* CHDLC_OFF = 4 */
      type = ntohs(chdlc->proto_code);
    } else {
      ip_offset = 2;
      type = ntohs(*((u_int16_t*)&packet[eth_offset]));
    }
    break;