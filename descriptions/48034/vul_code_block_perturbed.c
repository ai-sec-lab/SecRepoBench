if((x+2) >= packet->payload_packet_len) {
	    break;
	  }

	  responsetype = get16(&x, packet->payload);
	  rsp_ttl  = ntohl(*((u_int32_t*)&packet->payload[x+2]));

	  if(rsp_ttl == 0)
	    ndpi_set_risk(ndpi_struct, flow, NDPI_DNS_SUSPICIOUS_TRAFFIC, "DNS Record with zero TTL");	  

#ifdef DNS_DEBUG
	  printf("[DNS] TTL = %u\n", rsp_ttl);
	  printf("[DNS] [response] response_type=%d\n", responsetype);
#endif

	  ndpi_check_dns_type(ndpi_struct, flow, responsetype);