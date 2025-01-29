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