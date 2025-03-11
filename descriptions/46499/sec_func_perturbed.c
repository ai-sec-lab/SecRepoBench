static void ndpi_check_tinc(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *connection_flow)
{
  struct ndpi_packet_struct *packet = &ndpi_struct->packet;
  const u_int8_t *packet_payload = packet->payload;
  u_int32_t payload_len = packet->payload_packet_len;
  
  if(packet->udp != NULL) {
    if(ndpi_struct->tinc_cache != NULL) {
      struct tinc_cache_entry tinc_cache_entry1 = {
        .src_address = packet->iph->saddr,
        .dst_address = packet->iph->daddr,
        .dst_port = packet->udp->dest
      };

      struct tinc_cache_entry tinc_cache_entry2 = {
        .src_address = packet->iph->daddr,
        .dst_address = packet->iph->saddr,
        .dst_port = packet->udp->source
      };

      if(cache_remove(ndpi_struct->tinc_cache, &tinc_cache_entry1, sizeof(tinc_cache_entry1)) == CACHE_NO_ERROR ||
	 cache_remove(ndpi_struct->tinc_cache, &tinc_cache_entry2, sizeof(tinc_cache_entry2)) == CACHE_NO_ERROR) {

        cache_remove(ndpi_struct->tinc_cache, &tinc_cache_entry1, sizeof(tinc_cache_entry1));
        cache_remove(ndpi_struct->tinc_cache, &tinc_cache_entry2, sizeof(tinc_cache_entry2));

	/* cache_free(ndpi_struct->tinc_cache); */

        NDPI_LOG_INFO(ndpi_struct, "found tinc udp connection\n");
        ndpi_set_detected_protocol(ndpi_struct, connection_flow, NDPI_PROTOCOL_TINC, NDPI_PROTOCOL_UNKNOWN, NDPI_CONFIDENCE_DPI_CACHE);
      }
    }
    
    NDPI_EXCLUDE_PROTO(ndpi_struct, connection_flow);
    return;
  } else if(packet->tcp != NULL) {
    if(payload_len == 0) {
      if(packet->tcp->syn == 1 && packet->tcp->ack == 0) {
        connection_flow->tinc_cache_entry.src_address = packet->iph->saddr;
        connection_flow->tinc_cache_entry.dst_address = packet->iph->daddr;
        connection_flow->tinc_cache_entry.dst_port = packet->tcp->dest;
      }
      return;
    }

    switch(connection_flow->tinc_state) {
    case 0:
    case 1:
      if(payload_len > 6 && memcmp(packet_payload, "0 ", 2) == 0 && packet_payload[2] != ' ') {
	u_int32_t i = 3;
	while(i < payload_len && packet_payload[i++] != ' ');
	if(i+3 == payload_len && memcmp((packet_payload+i), "17\n", 3) == 0) {
	  connection_flow->tinc_state++;
	  return;
	}
      }
      break;

    case 2:
    case 3:
      if(payload_len > 11 && memcmp(packet_payload, "1 ", 2) == 0 && packet_payload[2] != ' ') {
	u_int16_t i = 3;
	u_int8_t numbers_left = 4;
	while(numbers_left) {
	  while(i < payload_len && packet_payload[i] >= '0' && packet_payload[i] <= '9') {
	    i++;
	  }

	  if(i < payload_len && packet_payload[i++] == ' ') {
	    numbers_left--;
	  }
	  else break;
	}
          
	if(numbers_left) break;
          
	while(i < payload_len &&
	      ((packet_payload[i] >= '0' && packet_payload[i] <= '9') ||
	       (packet_payload[i] >= 'A' && packet_payload[i] <= 'Z'))) {
	  i++;
	}
          
	if(i < payload_len && packet_payload[i] == '\n') {
	  if(++connection_flow->tinc_state > 3) {
	    if(ndpi_struct->tinc_cache == NULL)
	      ndpi_struct->tinc_cache = cache_new(TINC_CACHE_MAX_SIZE);              

	    cache_add(ndpi_struct->tinc_cache, &(connection_flow->tinc_cache_entry), sizeof(connection_flow->tinc_cache_entry));
	    NDPI_LOG_INFO(ndpi_struct, "found tinc tcp connection\n");
	    ndpi_set_detected_protocol(ndpi_struct, connection_flow, NDPI_PROTOCOL_TINC, NDPI_PROTOCOL_UNKNOWN, NDPI_CONFIDENCE_DPI);
	  }
	  return;
	}
      }
      break;
      
    default: break;
    }
  }

  NDPI_EXCLUDE_PROTO(ndpi_struct, connection_flow);
}