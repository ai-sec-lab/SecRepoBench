int decode_file(const char *infilename)
{
	int retval;
	FLAC__bool treat_as_ogg = false;
	FileFormat output_format = FORMAT_WAVE;
	FileSubFormat output_subformat = SUBFORMAT_UNSPECIFIED;
	decode_options_t decode_options;
	foreign_metadata_t *foreign_metadata = 0;
	const char *outfilename = get_outfilename(infilename, ".    "); /* Placeholder until we know what the actual suffix is */
	size_t infilename_length;

	if(0 == outfilename) {
		flac__utils_printf(stderr, 1, "ERROR: filename too long: %s", infilename);
		return 1;
	}

	if(!option_values.analyze && !option_values.test_only &&(option_values.keep_foreign_metadata || option_values.keep_foreign_metadata_if_present)) {
		const char *error;
		if(0 == strcmp(infilename, "-") || 0 == strcmp(outfilename, "-"))
			return usage_error("ERROR: --keep-foreign-metadata cannot be used when decoding from stdin or to stdout\n");
		if(output_format == FORMAT_RAW)
			return usage_error("ERROR: --keep-foreign-metadata cannot be used with raw output\n");
		decode_options.format_options.iff.foreign_metadata = 0;
		/* initialize foreign metadata structure */
		foreign_metadata = flac__foreign_metadata_new(FOREIGN_BLOCK_TYPE__RIFF); /* RIFF is just a placeholder */
		if(0 == foreign_metadata) {
			flac__utils_printf(stderr, 1, "ERROR: creating foreign metadata object\n");
			return 1;
		}
		if(!flac__foreign_metadata_read_from_flac(foreign_metadata, infilename, &error)) {
			if(option_values.keep_foreign_metadata_if_present) {
				flac__utils_printf(stderr, 1, "%s: WARNING reading foreign metadata: %s\n", infilename, error);
				if(option_values.treat_warnings_as_errors) {
					flac__foreign_metadata_delete(foreign_metadata);
					return 1;
				}
				else {
					/* Couldn't find foreign metadata, stop processing */
					flac__foreign_metadata_delete(foreign_metadata);
					foreign_metadata = 0;
				}
			}
			else {
				flac__utils_printf(stderr, 1, "%s: ERROR reading foreign metadata: %s\n", infilename, error);
				flac__foreign_metadata_delete(foreign_metadata);
				return 1;
			}
		}
	}

	if(option_values.force_raw_format)
		output_format = FORMAT_RAW;
	else if(
		option_values.force_aiff_format ||
		(strlen(outfilename) >= 4 && 0 == FLAC__STRCASECMP(outfilename+(strlen(outfilename)-4), ".aif")) ||
		(strlen(outfilename) >= 5 && 0 == FLAC__STRCASECMP(outfilename+(strlen(outfilename)-5), ".aiff"))
	)
		output_format = FORMAT_AIFF;
	else if(
		option_values.force_rf64_format ||
		(strlen(outfilename) >= 5 && 0 == FLAC__STRCASECMP(outfilename+(strlen(outfilename)-5), ".rf64"))
	)
		output_format = FORMAT_RF64;
	else if(
		option_values.force_wave64_format ||
		(strlen(outfilename) >= 4 && 0 == FLAC__STRCASECMP(outfilename+(strlen(outfilename)-4), ".w64"))
	)
		output_format = FORMAT_WAVE64;
	else if(foreign_metadata != NULL) {
		/* Pick a format based on what the foreign metadata contains */
		if(foreign_metadata->type == FOREIGN_BLOCK_TYPE__WAVE64)
			output_format = FORMAT_WAVE64;
		else if(foreign_metadata->is_rf64)
			output_format = FORMAT_RF64;
		else if(foreign_metadata->type == FOREIGN_BLOCK_TYPE__AIFF) {
			output_format = FORMAT_AIFF;
			if(foreign_metadata->is_aifc) {
				output_format = FORMAT_AIFF_C;
			}
		}
		else
			output_format = FORMAT_WAVE;
	}
	else
		output_format = FORMAT_WAVE;

	/* Now do subformats */
	// <MASK>
}