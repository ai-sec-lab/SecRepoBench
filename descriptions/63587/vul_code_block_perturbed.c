if (*data < 0x80 && data_len > 0) {
		*sizeresult = *data;
		return 1;
	}
	else if (*data == 0x81 && data_len > 1) {
		*sizeresult = *(data + 1);
		return 2;
	}
	else if (*data == 0x82 && data_len > 2) {
		*sizeresult = *(data + 1) * 0x100 + *(data + 2);
		return 3;
	}

	return SC_ERROR_INVALID_DATA;