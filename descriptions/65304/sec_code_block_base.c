ja.client.num_signature_algorithms = ndpi_min(sa_size, MAX_NUM_JA);
		for(i=0, id=0; i<tot_signature_algorithms_len && s_offset+i<total_len; i += 2) {
		  ja.client.signature_algorithms[id++] = ntohs(*(u_int16_t*)&packet->payload[s_offset+i]);
		}
		
		for(i=0, id=0; i<tot_signature_algorithms_len && s_offset+i<total_len; i++) {
		  int rc = ndpi_snprintf(&ja.client.signature_algorithms_str[i*2],
					 sizeof(ja.client.signature_algorithms_str)-i*2,
					 "%02X", packet->payload[s_offset+i]);
		  if(rc < 0) break;
		}