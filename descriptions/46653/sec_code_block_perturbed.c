if (file_path->len < 2)
		LOG_FUNC_RETURN(ctx, SC_ERROR_INVALID_ARGUMENTS);
	filepathcopy.value[0] = file_path->value[file_path->len - 2];
	filepathcopy.value[1] = file_path->value[file_path->len - 1];
	filepathcopy.len = 2;

	/* Reselect file to delete if the parent DF was selected and it's not DF. */
/*
	if (file_type != SC_FILE_TYPE_DF)   {
		rv = sc_select_file(p15card->card, &path, &file);
		LOG_TEST_RET(ctx, rv, "cannot select file to delete");
	}
*/

	sc_log(ctx, "Now really delete file");
	rv = sc_delete_file(p15card->card, &filepathcopy);
	LOG_FUNC_RETURN(ctx, rv);