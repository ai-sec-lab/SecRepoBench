/* https://en.wikipedia.org/wiki/TZSP */
      u_int offset           = ip_offset+ip_len+sizeof(struct ndpi_udphdr);
      u_int8_t version       = packet[offset];
      u_int8_t ts_type       = packet[offset+1];
      u_int16_t encapsulates = ntohs(*((u_int16_t*)&packet[offset+2]));

      tunnel_type = ndpi_tzsp_tunnel;

      if((version == 1) && (ts_type == 0) && (encapsulates == 1)) {
	u_int8_t stop = 0;

	offset += 4;

	while((!stop) && (offset < header->caplen)) {
	  u_int8_t tag_type = packet[offset];
	  u_int8_t tag_len;

	  switch(tag_type) {
	  case 0: /* PADDING Tag */
	    tag_len = 1;
	    break;
	  case 1: /* END Tag */
	    tag_len = 1, stop = 1;
	    break;
	  default:
	    tag_len = packet[offset+1];
	    break;
	  }

	  offset += tag_len;

	  if(offset >= header->caplen)
	    return(nproto); /* Invalid packet */
	  else {
	    eth_offset = offset;
	    goto datalink_check;
	  }
	}
      }