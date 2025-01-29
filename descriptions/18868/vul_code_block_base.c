size_t len;
	int r;

	len = sizeof(*life_cycle);
	receive_buf = (u8 *)life_cycle;
	r = coolkey_apdu_io(card, COOLKEY_CLASS, COOLKEY_INS_GET_LIFE_CYCLE, 0, 0,
			NULL, 0, &receive_buf, &len, NULL, 0);
	if (r == sizeof(*life_cycle)) {
		return SC_SUCCESS;
	}

	len = 1;
	receive_buf = &life_cycle->life_cycle;
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