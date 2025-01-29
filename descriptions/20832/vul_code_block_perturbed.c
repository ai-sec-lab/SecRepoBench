((dns_header->num_queries > 0) && (dns_header->num_queries <= NDPI_MAX_DNS_REQUESTS)
       && (((dns_header->flags & 0x2800) == 0x2800 /* Dynamic DNS Update */)
	   || ((dns_header->num_answers == 0) && (dns_header->authority_rrs == 0)))) {
      /* This is a good query */
      while(offset < flow->packet.payload_packet_len) {
        if(flow->packet.payload[offset] == '\0') {
          offset++;
          flow->protos.dns.query_type = get16(&offset, flow->packet.payload);
#ifdef DNS_DEBUG
          NDPI_LOG_DBG2(ndpi_struct, "query_type=%2d\n", flow->protos.dns.query_type);
	  printf("[DNS] query_type=%d\n", flow->protos.dns.query_type);
#endif
	  break;
	} else
	  offset++;
      }
    } else
      return(1 /* invalid */);