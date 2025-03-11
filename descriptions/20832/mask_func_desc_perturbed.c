static int search_valid_dns(struct ndpi_detection_module_struct *ndpi_struct,
			    struct ndpi_flow_struct *flow,
			    struct ndpi_dns_packet_header *dns_header,
			    int payload_offset, u_int8_t *is_query) {
  int offset = payload_offset;

  memcpy(dns_header, (struct ndpi_dns_packet_header*)&flow->packet.payload[offset],
	 sizeof(struct ndpi_dns_packet_header));

  dns_header->tr_id = ntohs(dns_header->tr_id);
  dns_header->flags = ntohs(dns_header->flags);
  dns_header->num_queries = ntohs(dns_header->num_queries);
  dns_header->num_answers = ntohs(dns_header->num_answers);
  dns_header->authority_rrs = ntohs(dns_header->authority_rrs);
  dns_header->additional_rrs = ntohs(dns_header->additional_rrs);

  offset += sizeof(struct ndpi_dns_packet_header);

  /* 0x0000 QUERY */
  if((dns_header->flags & FLAGS_MASK) == 0x0000)
    *is_query = 1;
  /* 0x8000 RESPONSE */
  else if((dns_header->flags & FLAGS_MASK) == 0x8000)
    *is_query = 0;
  else
    return(1 /* invalid */);

  if(*is_query) {
    /* DNS Request */
    if((dns_header->num_queries > 0) && (dns_header->num_queries <= NDPI_MAX_DNS_REQUESTS)
       && (((dns_header->flags & 0x2800) == 0x2800 /* Dynamic DNS Update */)
	   || ((dns_header->num_answers == 0) && (dns_header->authority_rrs == 0)))) 
	   // Extract the query type from the DNS query section of the packet.
	   // Iterate over the payload starting from the current offset until a null character is found.
	   // Once the null character is found, increment the offset and read the query type.
	   // Optionally, log the query type for debugging purposes.
	   // Ensure the operation is performed within the packet's payload length.
	   // <MASK>
 else
      return(1 /* invalid */);
  } else {
    /* DNS Reply */
    flow->protos.dns.reply_code = dns_header->flags & 0x0F;

    if((dns_header->num_queries > 0) && (dns_header->num_queries <= NDPI_MAX_DNS_REQUESTS) /* Don't assume that num_queries must be zero */
       && (((dns_header->num_answers > 0) && (dns_header->num_answers <= NDPI_MAX_DNS_REQUESTS))
	   || ((dns_header->authority_rrs > 0) && (dns_header->authority_rrs <= NDPI_MAX_DNS_REQUESTS))
	   || ((dns_header->additional_rrs > 0) && (dns_header->additional_rrs <= NDPI_MAX_DNS_REQUESTS)))
       ) {
      /* This is a good reply: we dissect it both for request and response */

      /* Leave the statement below commented necessary in case of call to ndpi_get_partial_detection() */
      offset++;

      if(flow->packet.payload[offset] != '\0') {
	while((offset < flow->packet.payload_packet_len)
	      && (flow->packet.payload[offset] != '\0')) {
	  offset++;
	}

	offset++;
      }

      offset += 4;

      if(dns_header->num_answers > 0) {
	u_int16_t rsp_type;
	u_int16_t num;

	for(num = 0; num < dns_header->num_answers; num++) {
	  u_int16_t data_len;

	  if((offset+6) >= flow->packet.payload_packet_len) {
	    break;
	  }

	  if((data_len = getNameLength(offset, flow->packet.payload, flow->packet.payload_packet_len)) == 0) {
	    break;
	  } else
	    offset += data_len;

	  if((offset+2) >= flow->packet.payload_packet_len) {
	    break;
	  }
	  rsp_type = get16(&offset, flow->packet.payload);
	  flow->protos.dns.rsp_type = rsp_type;

	  /* here x points to the response "class" field */
	  if((offset+12) <= flow->packet.payload_packet_len) {
	    offset += 6;
	    data_len = get16(&offset, flow->packet.payload);
	    
	    if((offset + data_len) <= flow->packet.payload_packet_len) {
	      // printf("[rsp_type: %u][data_len: %u]\n", rsp_type, data_len);

	      if(rsp_type == 0x05 /* CNAME */) {
		offset += data_len;
		continue; /* Skip CNAME */
	      }
	      
	      if((((rsp_type == 0x1) && (data_len == 4)) /* A */
#ifdef NDPI_DETECTION_SUPPORT_IPV6
		  || ((rsp_type == 0x1c) && (data_len == 16)) /* AAAA */
#endif
		  )) {
		memcpy(&flow->protos.dns.rsp_addr, flow->packet.payload + offset, data_len);
	      }
	    }
	  }
	  
	  break;
	}
      }
     
      if((flow->packet.detected_protocol_stack[0] == NDPI_PROTOCOL_DNS)
	 || (flow->packet.detected_protocol_stack[1] == NDPI_PROTOCOL_DNS)) {
	/* Request already set the protocol */
	// flow->extra_packets_func = NULL; /* Removed so the caller can keep dissecting DNS flows */
      } else {
	/* We missed the request */
	u_int16_t s_port = flow->packet.udp ? ntohs(flow->packet.udp->source) : ntohs(flow->packet.tcp->source);
	
	ndpi_set_detected_protocol(ndpi_struct, flow,
				   (s_port == 5355) ? NDPI_PROTOCOL_LLMNR : NDPI_PROTOCOL_DNS,
				   NDPI_PROTOCOL_UNKNOWN);
      }
    } else
      return(1 /* invalid */);
  }

  /* Valid */
  return(0);
}