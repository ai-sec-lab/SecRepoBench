static int
sc_oberthur_parse_privateinfo (struct sc_pkcs15_card *p15card,
		unsigned char *buff, size_t len, int postpone_allowed)
{
	struct sc_context *ctx = p15card->card->ctx;
	size_t ii;
	int rv;
	int privatekeysprocessed = 0, no_more_private_data = 0;

	LOG_FUNC_CALLED(ctx);

	for (ii=0; ii+5<=len; ii+=5)   {
		unsigned int file_id, size;

		if(*(buff+ii) != 0xFF)
			continue;

		file_id = 0x100 * *(buff+ii + 1) + *(buff+ii + 2);
		size = 0x100 * *(buff+ii + 3) + *(buff+ii + 4);
		sc_log(ctx, "add private object (file-id:%04X, size:%X)", file_id, size);

		switch (*(buff+ii + 1))   {
		case BASE_ID_PRV_RSA :
			if (privatekeysprocessed)
				break;

			rv = sc_pkcs15emu_oberthur_add_prvkey(p15card, file_id, size);
			if (rv == SC_ERROR_SECURITY_STATUS_NOT_SATISFIED && postpone_allowed)   {
				struct sc_path path;

				sc_log(ctx, "postpone adding of the private keys");
				sc_format_path("5011A5A5", &path);
				rv = sc_pkcs15_add_df(p15card, SC_PKCS15_PRKDF, &path);
				LOG_TEST_RET(ctx, rv, "Add PrkDF error");
				privatekeysprocessed = 1;
			}
			LOG_TEST_RET(ctx, rv, "Cannot parse private key info");
			break;
		case BASE_ID_PRV_DES :
			break;
		case BASE_ID_PRV_DATA :
			sc_log(ctx, "*(buff+ii + 1):%X", *(buff+ii + 1));
			if (no_more_private_data)
				break;

			rv = sc_pkcs15emu_oberthur_add_data(p15card, file_id, size, 1);
			if (rv == SC_ERROR_SECURITY_STATUS_NOT_SATISFIED && postpone_allowed)   {
				struct sc_path path;

				sc_log(ctx, "postpone adding of the private data");
				sc_format_path("5011A6A6", &path);
				rv = sc_pkcs15_add_df(p15card, SC_PKCS15_DODF, &path);
				LOG_TEST_RET(ctx, rv, "Add DODF error");
				no_more_private_data = 1;
			}
			LOG_TEST_RET(ctx, rv, "Cannot parse private data info");
			break;
		default:
			LOG_TEST_RET(ctx, SC_ERROR_UNKNOWN_DATA_RECEIVED, "Private object parse error");
		}
	}

	LOG_FUNC_RETURN(ctx, SC_SUCCESS);
}