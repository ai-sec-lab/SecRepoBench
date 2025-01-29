{
		if(datalen>48) {
			sc_log(card->ctx, "Data to be signed is too long (TCOS supports max. 48 bytes)\n");
			SC_FUNC_RETURN(card->ctx, SC_LOG_DEBUG_VERBOSE, SC_ERROR_INVALID_ARGUMENTS);
		}
		sc_format_apdu(card, &apdu, SC_APDU_CASE_4_SHORT, 0x2A, 0x9E, 0x9A);
		memcpy(sbuf, data, datalen);
		dlen=datalen;
	} else {
		int keylen= istcosv ? 256 : 128;
		sc_format_apdu(card, &apdu, keylen>255 ? SC_APDU_CASE_4_EXT : SC_APDU_CASE_4_SHORT, 0x2A,0x80,0x86);
		for(i=0; i<sizeof(sbuf);++i) sbuf[i]=0xff;
		if (keylen < datalen)
			return SC_ERROR_INVALID_ARGUMENTS;
		sbuf[0]=0x02; sbuf[1]=0x00; sbuf[2]=0x01; sbuf[keylen-datalen]=0x00;
		memcpy(sbuf+keylen-datalen+1, data, datalen);
		dlen=keylen+1;
	}