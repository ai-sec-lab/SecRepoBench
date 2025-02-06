else if(extension_id == 65486 /* encrypted server name */) {
		/*
		   - https://tools.ietf.org/html/draft-ietf-tls-esni-06
		   - https://blog.cloudflare.com/encrypted-sni/
		*/
		int e_offset = offset+extension_offset;
		int e_sni_len;
		int initial_offset = e_offset;
		u_int16_t cipher_suite = ntohs(*((u_int16_t*)&packet->payload[e_offset]));

		flow->protos.tls_quic.encrypted_sni.cipher_suite = cipher_suite;

		e_offset += 2; /* Cipher suite len */

		/* Key Share Entry */
		e_offset += 2; /* Group */
		if(e_offset + 2 < packet->payload_packet_len) {
		e_offset += ntohs(*((u_int16_t*)&packet->payload[e_offset])) + 2; /* Lenght */

		if((e_offset+4) < packet->payload_packet_len) {
		  /* Record Digest */
		  e_offset +=  ntohs(*((u_int16_t*)&packet->payload[e_offset])) + 2; /* Lenght */

		  if((e_offset+4) < packet->payload_packet_len) {
		    e_sni_len = ntohs(*((u_int16_t*)&packet->payload[e_offset]));
		    e_offset += 2;

		    if((e_offset+e_sni_len-(int)extension_len-initial_offset) >= 0 &&
		        e_offset+e_sni_len < packet->payload_packet_len) {
#ifdef DEBUG_ENCRYPTED_SNI
		      printf("Client TLS [Encrypted Server Name len: %u]\n", e_sni_len);
#endif

		      if(flow->protos.tls_quic.encrypted_sni.esni == NULL) {
			flow->protos.tls_quic.encrypted_sni.esni = (char*)ndpi_malloc(e_sni_len*2+1);

			if(flow->protos.tls_quic.encrypted_sni.esni) {
			  u_int16_t off;
			  int i;

			  for(i=e_offset, off=0; i<(e_offset+e_sni_len); i++) {
			    int rc = sprintf(&flow->protos.tls_quic.encrypted_sni.esni[off], "%02X", packet->payload[i] & 0XFF);

			    if(rc <= 0) {
			      flow->protos.tls_quic.encrypted_sni.esni[off] = '\0';
			      break;
			    } else
			      off += rc;
			  }
			}
		      }
		    }
		  }
		}
		}
	      }