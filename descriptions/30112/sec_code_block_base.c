if (offs + 2 > info_len) {
		free(info_blob);
		LOG_TEST_RET(ctx, SC_ERROR_UNKNOWN_DATA_RECEIVED, "Failed to add data: no 'label'");
	}
	label = info_blob + offs + 2;
	label_len = *(info_blob + offs + 1) + *(info_blob + offs) * 0x100;
	if (offs + 2 + label_len > info_len) {
		free(info_blob);
		LOG_TEST_RET(ctx, SC_ERROR_UNKNOWN_DATA_RECEIVED, "Invalid length of 'label' received");
	}
	if (label_len > sizeof(dobj.label) - 1)
		label_len = sizeof(dobj.label) - 1;
	offs += 2 + *(info_blob + offs + 1);

	/* Application */
	if (offs > info_len) {
		free(info_blob);
		LOG_TEST_RET(ctx, SC_ERROR_UNKNOWN_DATA_RECEIVED, "Failed to add data: no 'application'");
	}
	app = info_blob + offs + 2;
	app_len = *(info_blob + offs + 1) + *(info_blob + offs) * 0x100;
	if (app_len > sizeof(dinfo.app_label) - 1)
		app_len = sizeof(dinfo.app_label) - 1;
	offs += 2 + app_len;

	/* OID encode like DER(ASN.1(oid)) */
	if (offs + 1 > info_len) {
		free(info_blob);
		LOG_TEST_RET(ctx, SC_ERROR_UNKNOWN_DATA_RECEIVED, "Failed to add data: no 'OID'");
	}