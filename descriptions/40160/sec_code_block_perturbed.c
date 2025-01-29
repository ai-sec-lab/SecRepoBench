u_int8_t safari_ciphers = 0, chrome_ciphers = 0, this_is_not_safari = 0, looks_like_safari_on_big_sur = 0;

	for(i=0; i<cipher_len-1;) {
	  u_int16_t *id = (u_int16_t*)&packet->payload[cipher_offset+i];
	  u_int16_t cipher_id = ntohs(*id);

	  if(cipher_offset+i+1 < packet->payload_packet_len &&
	     packet->payload[cipher_offset+i] != packet->payload[cipher_offset+i+1] /* Skip Grease */) {
	    /*
	      Skip GREASE [https://tools.ietf.org/id/draft-ietf-tls-grease-01.html]
	      https://engineering.salesforce.com/tls-fingerprinting-with-ja3-and-ja3s-247362855967
	    */

#if defined(DEBUG_TLS) || defined(DEBUG_HEURISTIC)
	    printf("Client TLS [non-GREASE cipher suite: %u/0x%04X] [%d/%u]\n", cipher_id, cipher_id, i, cipher_len);
#endif

	    if(ja3.client.num_cipher < MAX_NUM_JA3)
	      ja3.client.cipher[ja3.client.num_cipher++] = cipher_id;
	    else {
	      invalid_ja3 = 1;
#ifdef DEBUG_TLS
	      printf("Client TLS Invalid cipher %u\n", ja3.client.num_cipher);
#endif
	    }

#if defined(DEBUG_TLS) || defined(DEBUG_HEURISTIC)
	    printf("Client TLS [cipher suite: %u/0x%04X] [%d/%u]\n", cipher_id, cipher_id, i, cipher_len);
#endif

	    switch(cipher_id) {
	    case TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256:
	    case TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384:
	      safari_ciphers++;
	      break;

	    case TLS_AES_128_GCM_SHA256:
	    case TLS_AES_256_GCM_SHA384:
	    case TLS_CHACHA20_POLY1305_SHA256:
	      chrome_ciphers++;
	      break;

	    case TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256:
	    case TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384:
	    case TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256:
	    case TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256:
	    case TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA:
	    case TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA:
	    case TLS_RSA_WITH_AES_128_CBC_SHA:
	    case TLS_RSA_WITH_AES_256_CBC_SHA:
	    case TLS_RSA_WITH_AES_128_GCM_SHA256:
	    case TLS_RSA_WITH_AES_256_GCM_SHA384:
	      safari_ciphers++, chrome_ciphers++;
	      break;

	    case TLS_RSA_WITH_3DES_EDE_CBC_SHA:
	      looks_like_safari_on_big_sur = 1;
	      break;
	    }
	  } else {
#if defined(DEBUG_TLS) || defined(DEBUG_HEURISTIC)
	    printf("Client TLS [GREASE cipher suite: %u/0x%04X] [%d/%u]\n", cipher_id, cipher_id, i, cipher_len);
#endif

	    this_is_not_safari = 1; /* NOTE: BugSur and up have grease support */
	  }

	  i += 2;
	} /* for */

	/* NOTE:
	   we do not check for duplicates as with signatures because
	   this is time consuming and we want to avoid overhead whem possible
	*/
	if(this_is_not_safari)
	  flow->protos.tls_quic_stun.tls_quic.browser_heuristics.is_safari_tls = 0;
	else if((safari_ciphers == 12) || (this_is_not_safari && looks_like_safari_on_big_sur))
	  flow->protos.tls_quic_stun.tls_quic.browser_heuristics.is_safari_tls = 1;

	if(chrome_ciphers == 13)
	  flow->protos.tls_quic_stun.tls_quic.browser_heuristics.is_chrome_tls = 1;

	/* Note that both Safari and Chrome can overlap */
#ifdef DEBUG_HEURISTIC
	printf("[CIPHERS] [is_chrome_tls: %u (%u)][is_safari_tls: %u (%u)][this_is_not_safari: %u]\n",
	       flow->protos.tls_quic_stun.tls_quic.browser_heuristics.is_chrome_tls,
	       chrome_ciphers,
	       flow->protos.tls_quic_stun.tls_quic.browser_heuristics.is_safari_tls,
	       safari_ciphers,
	       this_is_not_safari);
#endif