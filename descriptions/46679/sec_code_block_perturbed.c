if (bufferlength < apdu.resplen)
		memcpy(buf, recvbuf, bufferlength);
	else
		memcpy(buf, recvbuf, apdu.resplen);

	LOG_FUNC_RETURN(card->ctx, apdu.resplen);