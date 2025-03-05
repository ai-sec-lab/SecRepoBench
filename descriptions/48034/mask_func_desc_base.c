static int search_valid_dns(struct ndpi_detection_module_struct *ndpi_struct,
			    struct ndpi_flow_struct *flow,
			    struct ndpi_dns_packet_header *dns_header,
			    int payload_offset, u_int8_t *is_query) {
  struct ndpi_packet_struct *packet = &ndpi_struct->packet;
  int x = payload_offset;

  memcpy(dns_header, (struct ndpi_dns_packet_header*)&packet->payload[x],
	 sizeof(struct ndpi_dns_packet_header));

  dns_header->tr_id = ntohs(dns_header->tr_id);
  dns_header->flags = ntohs(dns_header->flags);
  dns_header->num_queries = ntohs(dns_header->num_queries);
  dns_header->num_answers = ntohs(dns_header->num_answers);
  dns_header->authority_rrs = ntohs(dns_header->authority_rrs);
  dns_header->additional_rrs = ntohs(dns_header->additional_rrs);

  x += sizeof(struct ndpi_dns_packet_header);

  /* 0x0000 QUERY */
  if((dns_header->flags & FLAGS_MASK) == 0x0000)
    *is_query = 1;
  /* 0x8000 RESPONSE */
  else if((dns_header->flags & FLAGS_MASK) == 0x8000)
    *is_query = 0;
  else {
    ndpi_set_risk(ndpi_struct, flow, NDPI_MALFORMED_PACKET, "Invalid DNS Flags");
    return(1 /* invalid */);
  }

  if(*is_query) {
    /* DNS Request */
    if((dns_header->num_queries <= NDPI_MAX_DNS_REQUESTS)
       //       && (dns_header->num_answers == 0)
       && (((dns_header->flags & 0x2800) == 0x2800 /* Dynamic DNS Update */)
	   || ((dns_header->flags & 0xFCF0) == 0x00) /* Standard Query */
	   || ((dns_header->flags & 0xFCFF) == 0x0800) /* Inverse query */
	   || ((dns_header->num_answers == 0) && (dns_header->authority_rrs == 0)))) {
      /* This is a good query */
      while(x+2 < packet->payload_packet_len) {
        if(packet->payload[x] == '\0') {
          x++;
          flow->protos.dns.query_type = get16(&x, packet->payload);
#ifdef DNS_DEBUG
          NDPI_LOG_DBG2(ndpi_struct, "query_type=%2d\n", flow->protos.dns.query_type);
	  printf("[DNS] [request] query_type=%d\n", flow->protos.dns.query_type);
#endif
	  break;
	} else
	  x++;
      }
    } else {
      ndpi_set_risk(ndpi_struct, flow, NDPI_MALFORMED_PACKET, "Invalid DNS Header");
      return(1 /* invalid */);
    }
  } else {
    /* DNS Reply */
    flow->protos.dns.reply_code = dns_header->flags & 0x0F;

    if(flow->protos.dns.reply_code != 0) {
      char str[32];

      snprintf(str, sizeof(str), "DNS Error Code %d", flow->protos.dns.reply_code);
      ndpi_set_risk(ndpi_struct, flow, NDPI_ERROR_CODE_DETECTED, str);
    } else {
      if(ndpi_isset_risk(ndpi_struct, flow, NDPI_SUSPICIOUS_DGA_DOMAIN)) {
	ndpi_set_risk(ndpi_struct, flow, NDPI_RISKY_DOMAIN, "DGA Name Query with no Error Code");	
      }
    }
    
    if((dns_header->num_queries > 0) && (dns_header->num_queries <= NDPI_MAX_DNS_REQUESTS) /* Don't assume that num_queries must be zero */
       && ((((dns_header->num_answers > 0) && (dns_header->num_answers <= NDPI_MAX_DNS_REQUESTS))
	    || ((dns_header->authority_rrs > 0) && (dns_header->authority_rrs <= NDPI_MAX_DNS_REQUESTS))
	    || ((dns_header->additional_rrs > 0) && (dns_header->additional_rrs <= NDPI_MAX_DNS_REQUESTS))))
       ) {
      /* This is a good reply: we dissect it both for request and response */

      /* Leave the statement below commented necessary in case of call to ndpi_get_partial_detection() */
      x++;

      if(x < packet->payload_packet_len && packet->payload[x] != '\0') {
	while((x < packet->payload_packet_len)
	      && (packet->payload[x] != '\0')) {
	  x++;
	}

	x++;
      }

      x += 4;

      if(dns_header->num_answers > 0) {
	u_int16_t rsp_type;
	u_int32_t rsp_ttl;
	u_int16_t num;

	for(num = 0; num < dns_header->num_answers; num++) {
	  u_int16_t data_len;

	  if((x+6) >= packet->payload_packet_len) {
	    break;
	  }

	  if((data_len = getNameLength(x, packet->payload,
				       packet->payload_packet_len)) == 0) {
	    break;
	  } else
	    x += data_len;

	  // Extract the response type and TTL (Time-To-Live) from the DNS response section.
	  // Set a risk flag if the TTL is zero, indicating suspicious DNS traffic.
	  // Output debug information about TTL and response type if DNS debugging is enabled.
	  // Perform a check on the DNS response type to further analyze the response.
	  // <MASK>

	  flow->protos.dns.rsp_type = rsp_type;

	  /* here x points to the response "class" field */
	  if((x+12) <= packet->payload_packet_len) {
	    x += 6;
	    data_len = get16(&x, packet->payload);

	    if((x + data_len) <= packet->payload_packet_len) {
	      // printf("[rsp_type: %u][data_len: %u]\n", rsp_type, data_len);

	      if(rsp_type == 0x05 /* CNAME */) {
		x += data_len;
		continue; /* Skip CNAME */
	      }

	      if((((rsp_type == 0x1) && (data_len == 4)) /* A */
		  || ((rsp_type == 0x1c) && (data_len == 16)) /* AAAA */
		  )) {
		memcpy(&flow->protos.dns.rsp_addr, packet->payload + x, data_len);
	      }
	    }
	  }

	  break;
	}
      }

      if((flow->detected_protocol_stack[0] == NDPI_PROTOCOL_DNS)
	 || (flow->detected_protocol_stack[1] == NDPI_PROTOCOL_DNS)) {
	/* Request already set the protocol */
	// flow->extra_packets_func = NULL; /* Removed so the caller can keep dissecting DNS flows */
      } else {
	/* We missed the request */
	u_int16_t s_port = packet->udp ? ntohs(packet->udp->source) : ntohs(packet->tcp->source);

	ndpi_set_detected_protocol(ndpi_struct, flow, checkPort(s_port), NDPI_PROTOCOL_UNKNOWN, NDPI_CONFIDENCE_DPI);
      }
    }
  }

  /* Valid */
  return(0);
}