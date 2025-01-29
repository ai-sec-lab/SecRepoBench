if (rc < 0)
		LOG_FUNC_RETURN(card->ctx, rc);

	/* We expect this will fill the whole structure in the argument.
	 * If we got something else, report error */
	if ((size_t)apdu.resplen < sizeof(cplc_data)) {
		LOG_FUNC_RETURN(card->ctx, SC_ERROR_CORRUPTED_DATA);
	}
	LOG_FUNC_RETURN(card->ctx, apdu.resplen);