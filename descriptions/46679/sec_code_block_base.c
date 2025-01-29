if (buflen < apdu.resplen)
		memcpy(buf, recvbuf, buflen);
	else
		memcpy(buf, recvbuf, apdu.resplen);

	LOG_FUNC_RETURN(card->ctx, apdu.resplen);