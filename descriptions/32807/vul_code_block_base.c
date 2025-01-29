if (offs + 2 + oid_len > info_len) {
		free(info_blob);
		LOG_TEST_RET(ctx, SC_ERROR_UNKNOWN_DATA_RECEIVED, "Invalid length of 'oid' received");
	}
	if (oid_len)   {
		oid = info_blob + offs + 2;
		if (*oid != 0x06 || (*(oid + 1) != oid_len - 2)) {
			free(info_blob);
			LOG_TEST_RET(ctx, SC_ERROR_UNKNOWN_DATA_RECEIVED, "Failed to add data: invalid 'OID' format");
		}
		oid += 2;
		oid_len -= 2;
	}

	snprintf(ch_tmp, sizeof(ch_tmp), "%s%04X", private ? AWP_OBJECTS_DF_PRV : AWP_OBJECTS_DF_PUB, file_id);

	sc_format_path(ch_tmp, &dinfo.path);

	memcpy(dobj.label, label, label_len);
	memcpy(dinfo.app_label, app, app_len);
	if (oid_len)
		sc_asn1_decode_object_id(oid, oid_len, &dinfo.app_oid);

	if (flags & OBERTHUR_ATTR_MODIFIABLE)
		dobj.flags |= SC_PKCS15_CO_FLAG_MODIFIABLE;

	if (private)   {
		dobj.auth_id.len = sizeof(PinDomainID) > sizeof(dobj.auth_id.value)
				? sizeof(dobj.auth_id.value) : sizeof(PinDomainID);
		memcpy(dobj.auth_id.value, PinDomainID, dobj.auth_id.len);

		dobj.flags |= SC_PKCS15_CO_FLAG_PRIVATE;
	}

	rv = sc_pkcs15emu_add_data_object(p15card, &dobj, &dinfo);

	free(info_blob);
	LOG_FUNC_RETURN(p15card->card->ctx, rv);