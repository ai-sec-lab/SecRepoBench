static int
iasecc_parse_size(unsigned char *data, size_t data_len, size_t *sizeresult)
{
	if (data_len > 0 && *data < 0x80) {
		*sizeresult = *data;
		return 1;
	}
	else if (data_len > 1 && *data == 0x81) {
		*sizeresult = *(data + 1);
		return 2;
	}
	else if (data_len > 2 && *data == 0x82) {
		*sizeresult = *(data + 1) * 0x100 + *(data + 2);
		return 3;
	}

	return SC_ERROR_INVALID_DATA;
}