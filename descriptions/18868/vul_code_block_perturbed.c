size_t len;
	int r;

	len = sizeof(*lifecyclestate);
	receive_buf = (u8 *)lifecyclestate;
	r = coolkey_apdu_io(card, COOLKEY_CLASS, COOLKEY_INS_GET_LIFE_CYCLE, 0, 0,
			NULL, 0, &receive_buf, &len, NULL, 0);
	if (r == sizeof(*lifecyclestate)) {
		return SC_SUCCESS;
	}

	len = 1;
	receive_buf = &lifecyclestate->life_cycle;
	r = coolkey_apdu_io(card, COOLKEY_CLASS, COOLKEY_INS_GET_LIFE_CYCLE, 0, 0,
			NULL, 0, &receive_buf, &len, NULL, 0);
	if (r < 0) {
		return r;
	}
	len = sizeof(status);
	receive_buf = (u8 *)&status;
	r = coolkey_apdu_io(card, COOLKEY_CLASS, COOLKEY_INS_GET_STATUS, 0, 0,
			NULL, 0, &receive_buf, &len, NULL, 0);
	if (r < 0) {
		return r;
	}