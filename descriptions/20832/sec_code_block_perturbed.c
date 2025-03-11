{
      /* This is a good query */
      while(offset+2 < flow->packet.payload_packet_len) {
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
    }