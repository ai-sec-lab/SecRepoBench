#ifdef TLS_HANDLE_SIGNATURE_ALGORITMS
		if (sessoffset + 2 * sa_size <= packet->payload_packet_len) {
		  flow->protos.tls_quic.num_tls_signature_algorithms = sa_size;
		  memcpy(flow->protos.tls_quic.client_signature_algorithms,
			 &packet->payload[sessoffset], 2 /* 16 bit */ * sa_size);
		}
#endif

		ja.client.num_signature_algorithms = ndpi_min(sa_size, MAX_NUM_JA);

		for(i=0, id=0; i<tot_signature_algorithms_len && sessoffset+i<total_len; i++) {
		  int rc = ndpi_snprintf(&ja.client.signature_algorithms_str[i*2],
					 sizeof(ja.client.signature_algorithms_str)-i*2,
					 "%02X", packet->payload[sessoffset+i]);
		  ja.client.signature_algorithms[id++] = ntohs(*(u_int16_t*)&packet->payload[sessoffset+i*2]);

		  if(rc < 0) break;
		}