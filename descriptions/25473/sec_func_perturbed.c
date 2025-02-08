static int itacns_add_data_files(sc_pkcs15_card_t *p15card)
{
	const size_t array_size =
		sizeof(itacns_data_files)/sizeof(itacns_data_files[0]);
	unsigned int i;
	int rv;
	sc_pkcs15_data_t *p15_personaldata = NULL;
	sc_pkcs15_data_info_t dinfo;
	struct sc_pkcs15_object *objs[32];
	struct sc_pkcs15_data_info *cinfo;

	for(i=0; i < array_size; i++) {
		sc_path_t filepath;
		sc_pkcs15_data_info_t data;
		sc_pkcs15_object_t    obj;

		if (itacns_data_files[i].cie_only &&
			p15card->card->type != SC_CARD_TYPE_ITACNS_CIE_V2)
			continue;

		sc_format_path(itacns_data_files[i].path, &filepath);

		memset(&data, 0, sizeof(data));
		memset(&obj, 0, sizeof(obj));
		strlcpy(data.app_label, itacns_data_files[i].label,
			sizeof(data.app_label));
		strlcpy(obj.label, itacns_data_files[i].label,
			sizeof(obj.label));
		data.path = filepath;
		rv = sc_pkcs15emu_add_data_object(p15card, &obj, &data);
		LOG_TEST_RET(p15card->card->ctx, rv,
			"Could not add data file");
	}

	/*
	 * If we got this far, we can read the Personal Data file and glean
	 * the user's full name. Thus we can use it to put together a
	 * user-friendlier card name.
	 */
	memset(&dinfo, 0, sizeof(dinfo));
	strlcpy(dinfo.app_label, "EF_DatiPersonali", sizeof(dinfo.app_label));

	/* Find EF_DatiPersonali */

	rv = sc_pkcs15_get_objects(p15card, SC_PKCS15_TYPE_DATA_OBJECT,
		objs, 32);
	if(rv < 0) {
		sc_log(p15card->card->ctx,
			"Data enumeration failed");
		return SC_SUCCESS;
	}

	for(i=0; i<32; i++) {
		cinfo = (struct sc_pkcs15_data_info *) objs[i]->data;
		if(!strcmp("EF_DatiPersonali", objs[i]->label))
			break;
	}

	if(i>=32) {
		sc_log(p15card->card->ctx,
			"Could not find EF_DatiPersonali: "
			"keeping generic card name");
		return SC_SUCCESS;
	}

	rv = sc_pkcs15_read_data_object(p15card, cinfo, &p15_personaldata);
	if (rv) {
		sc_log(p15card->card->ctx,
			"Could not read EF_DatiPersonali: "
			"keeping generic card name");
		return SC_SUCCESS;
	}

	if (p15_personaldata->data) {
		char fullname[160];
		if (get_name_from_EF_DatiPersonali(p15_personaldata->data,
			p15_personaldata->data_len, fullname, sizeof(fullname))) {
			sc_log(p15card->card->ctx,
				"Could not parse EF_DatiPersonali: "
				"keeping generic card name");
			sc_pkcs15_free_data_object(p15_personaldata);
			free(cinfo->data.value);
			cinfo->data.value = NULL;
			return SC_SUCCESS;
		}
		set_string(&p15card->tokeninfo->label, fullname);
	}
	free(cinfo->data.value);
	cinfo->data.value = NULL;
	sc_pkcs15_free_data_object(p15_personaldata);
	return SC_SUCCESS;
}