size_t receive_len;
	int len;

	receive_len = sizeof(*lifecyclestate);
	receive_buf = (u8 *)lifecyclestate;
	len = coolkey_apdu_io(card, COOLKEY_CLASS, COOLKEY_INS_GET_LIFE_CYCLE, 0, 0,
			NULL, 0, &receive_buf, &receive_len, NULL, 0);
	if (len == sizeof(*lifecyclestate)) {
		return SC_SUCCESS;
	}

	receive_len = 1;
	receive_buf = &lifecyclestate->life_cycle;
	len = coolkey_apdu_io(card, COOLKEY_CLASS, COOLKEY_INS_GET_LIFE_CYCLE, 0, 0,
			NULL, 0, &receive_buf, &receive_len, NULL, 0);
	if (len < 0) { /* Error from the trasmittion */
		return len;
	}
	if (len != 1) { /* The returned data is invalid */
		return SC_ERROR_INTERNAL;
	}
	receive_len = sizeof(status);
	receive_buf = (u8 *)&status;
	len = coolkey_apdu_io(card, COOLKEY_CLASS, COOLKEY_INS_GET_STATUS, 0, 0,
			NULL, 0, &receive_buf, &receive_len, NULL, 0);
	if (len < 0) { /* Error from the trasmittion */
		return len;
	}
	if (len != sizeof(status)) { /* The returned data is invalid */
		return SC_ERROR_INTERNAL;
	}