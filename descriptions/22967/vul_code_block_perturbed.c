memcpy(sbuf+1, ciphergram, crgram_len);

	r = sc_transmit_apdu(card, &apdu);
	LOG_TEST_RET(card->ctx, r, "APDU transmit failed");

	if (apdu.sw1==0x90 && apdu.sw2==0x00) {
		size_t len= (apdu.resplen>outlen) ? outlen : apdu.resplen;
		unsigned int offset=0;
		if(tcos3 && (data->pad_flags & SC_ALGORITHM_RSA_PAD_PKCS1) && apdu.resp[0]==0 && apdu.resp[1]==2) {
			offset=2; while(offset<len && apdu.resp[offset]!=0) ++offset;
			offset=(offset<len-1) ? offset+1 : 0;
		}
		memcpy(out, apdu.resp+offset, len-offset);
		SC_FUNC_RETURN(card->ctx, SC_LOG_DEBUG_VERBOSE, len-offset);
	}
	SC_FUNC_RETURN(card->ctx, SC_LOG_DEBUG_VERBOSE, sc_check_sw(card, apdu.sw1, apdu.sw2));