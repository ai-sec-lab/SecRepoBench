static int
coolkey_get_life_cycle(sc_card_t *card, coolkey_life_cycle_t *life_cycle)
{
	coolkey_status_t status;
	u8 *receive_buf;
	size_t receive_len;
	int len;

	receive_len = sizeof(*life_cycle);
	receive_buf = (u8 *)life_cycle;
	len = coolkey_apdu_io(card, COOLKEY_CLASS, COOLKEY_INS_GET_LIFE_CYCLE, 0, 0,
			NULL, 0, &receive_buf, &receive_len, NULL, 0);
	if (len == sizeof(*life_cycle)) {
		return SC_SUCCESS;
	}

	receive_len = 1;
	receive_buf = &life_cycle->life_cycle;
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
	life_cycle->protocol_version_major = status.protocol_version_major;
	life_cycle->protocol_version_minor = status.protocol_version_minor;
	life_cycle->pin_count = status.pin_count;
	return SC_SUCCESS;
}