return coolkey_apdu_io(card, GLOBAL_PLATFORM_CLASS, ISO7816_INS_GET_DATA, 0x9f, 0x7f,
			NULL, 0, &receive_buf, &length,  NULL, 0);