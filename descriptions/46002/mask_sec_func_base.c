static int
cosm_new_file(struct sc_profile *profile, struct sc_card *card,
	      unsigned int type, unsigned int num, struct sc_file **out)
{
	// <MASK>

	file->id &= 0xFF00;
	file->id |= (num & 0x00FF);

	file->path.value[file->path.len - 1] = (num & 0xFF);
	file->type = SC_FILE_TYPE_INTERNAL_EF;
	file->ef_structure = structure;

	sc_log(card->ctx, 
		 "file size %"SC_FORMAT_LEN_SIZE_T"u; ef type %i/%i; id %04X, path_len %"SC_FORMAT_LEN_SIZE_T"u\n",
		 file->size, file->type, file->ef_structure, file->id,
		 file->path.len);
	sc_log(card->ctx,  "file path: %s",
		 sc_print_path(&(file->path)));
	*out = file;

	SC_FUNC_RETURN(card->ctx, SC_LOG_DEBUG_VERBOSE, SC_SUCCESS);
}