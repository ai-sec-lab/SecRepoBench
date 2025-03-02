{
      /* This is a good query */
      while(x+2 < flow->packet.payload_packet_len) {
        if(flow->packet.payload[x] == '\0') {
          x++;
          flow->protos.dns.query_type = get16(&x, flow->packet.payload);
#ifdef DNS_DEBUG
          NDPI_LOG_DBG2(ndpi_struct, "query_type=%2d\n", flow->protos.dns.query_type);
	  printf("[DNS] query_type=%d\n", flow->protos.dns.query_type);
#endif
	  break;
	} else
	  x++;
      }
    }