if(option_values.force_legacy_wave_format)
		output_subformat = SUBFORMAT_WAVE_PCM;
	else if(option_values.force_extensible_wave_format)
		output_subformat = SUBFORMAT_WAVE_EXTENSIBLE;
	else if(option_values.force_aiff_c_none_format) {
		output_format = FORMAT_AIFF_C;
		output_subformat = SUBFORMAT_AIFF_C_NONE;
	}
	else if(option_values.force_aiff_c_sowt_format) {
		output_format = FORMAT_AIFF_C;
		output_subformat = SUBFORMAT_AIFF_C_SOWT;
	}
	else if(foreign_metadata != NULL) {
		if(foreign_metadata->is_wavefmtex)
			output_subformat = SUBFORMAT_WAVE_EXTENSIBLE;
		else if(output_format == FORMAT_WAVE)
			output_subformat = SUBFORMAT_WAVE_PCM;
		else if(foreign_metadata->is_aifc) {
			if(foreign_metadata->is_sowt)
				output_subformat = SUBFORMAT_AIFF_C_SOWT;
			else
				output_subformat = SUBFORMAT_AIFF_C_NONE;
		}
	}


	/* Check whether output format agrees with foreign metadata */
	if(foreign_metadata != NULL) {
		if((output_format != FORMAT_WAVE && output_format != FORMAT_RF64) && foreign_metadata->type == FOREIGN_BLOCK_TYPE__RIFF) {
			flac__foreign_metadata_delete(foreign_metadata);
			return usage_error("ERROR: foreign metadata type RIFF cannot be restored to a%s file, only to WAVE and RF64\n",FileFormatString[output_format]);
		}
		if((output_format != FORMAT_AIFF && output_format != FORMAT_AIFF_C) && foreign_metadata->type == FOREIGN_BLOCK_TYPE__AIFF) {
			flac__foreign_metadata_delete(foreign_metadata);
			return usage_error("ERROR: foreign metadata type AIFF cannot be restored to a%s file, only to AIFF and AIFF-C\n",FileFormatString[output_format]);
		}
		if(output_format != FORMAT_WAVE64 && foreign_metadata->type == FOREIGN_BLOCK_TYPE__WAVE64) {
			flac__foreign_metadata_delete(foreign_metadata);
			return usage_error("ERROR: foreign metadata type Wave64 cannot be restored to a%s file, only to Wave64\n",FileFormatString[output_format]);
		}
	}

	/* Now reassemble outfilename */
	get_decoded_outfilename(infilename, output_format);

	/*
	 * Error if output file already exists (and -f not used).
	 * Use grabbag__file_get_filesize() as a cheap way to check.
	 */
	if(!option_values.test_only && !option_values.force_file_overwrite && strcmp(outfilename, "-") && grabbag__file_get_filesize(outfilename) != (FLAC__off_t)(-1)) {
		flac__utils_printf(stderr, 1, "ERROR: output file %s already exists, use -f to override\n", outfilename);
		flac__foreign_metadata_delete(foreign_metadata);
		return 1;
	}

	if(!option_values.test_only && !option_values.analyze) {
		if(output_format == FORMAT_RAW && (option_values.format_is_big_endian < 0 || option_values.format_is_unsigned_samples < 0)) {
			flac__foreign_metadata_delete(foreign_metadata);
			return usage_error("ERROR: for decoding to a raw file you must specify a value for --endian and --sign\n");
		}
	}

	infilename_length = strlen(infilename);
	if(option_values.use_ogg)
		treat_as_ogg = true;
	else if(infilename_length >= 4 && 0 == FLAC__STRCASECMP(infilename+(infilename_length-4), ".oga"))
		treat_as_ogg = true;
	else if(infilename_length >= 4 && 0 == FLAC__STRCASECMP(infilename+(infilename_length-4), ".ogg"))
		treat_as_ogg = true;
	else
		treat_as_ogg = false;

#if !FLAC__HAS_OGG
	if(treat_as_ogg) {
		flac__utils_printf(stderr, 1, "%s: Ogg support has not been built into this copy of flac\n", infilename);
		flac__foreign_metadata_delete(foreign_metadata);
		return 1;
	}
#endif

	if(!flac__utils_parse_skip_until_specification(option_values.skip_specification, &decode_options.skip_specification) || decode_options.skip_specification.is_relative) {
		flac__foreign_metadata_delete(foreign_metadata);
		return usage_error("ERROR: invalid value for --skip\n");
	}

	if(!flac__utils_parse_skip_until_specification(option_values.until_specification, &decode_options.until_specification)) { /*@@@ more checks: no + without --skip, no - unless known total_samples_to_{en,de}code */
		flac__foreign_metadata_delete(foreign_metadata);
		return usage_error("ERROR: invalid value for --until\n");
	}
	/* if there is no "--until" we want to default to "--until=-0" */
	if(0 == option_values.until_specification)
		decode_options.until_specification.is_relative = true;

	if(option_values.cue_specification) {
		if(!flac__utils_parse_cue_specification(option_values.cue_specification, &decode_options.cue_specification)) {
			flac__foreign_metadata_delete(foreign_metadata);
			return usage_error("ERROR: invalid value for --cue\n");
		}
		decode_options.has_cue_specification = true;
	}
	else
		decode_options.has_cue_specification = false;

	decode_options.treat_warnings_as_errors = option_values.treat_warnings_as_errors;
	decode_options.continue_through_decode_errors = option_values.continue_through_decode_errors;
	decode_options.relaxed_foreign_metadata_handling = option_values.keep_foreign_metadata_if_present;
	decode_options.replaygain_synthesis_spec = option_values.replaygain_synthesis_spec;
	decode_options.force_subformat = output_subformat;
#if FLAC__HAS_OGG
	decode_options.is_ogg = treat_as_ogg;
	decode_options.use_first_serial_number = !option_values.has_serial_number;
	decode_options.serial_number = option_values.serial_number;
#endif
	decode_options.channel_map_none = option_values.channel_map_none;
	decode_options.format = output_format;

	if(output_format == FORMAT_RAW) {
		decode_options.format_options.raw.is_big_endian = option_values.format_is_big_endian;
		decode_options.format_options.raw.is_unsigned_samples = option_values.format_is_unsigned_samples;

		retval = flac__decode_file(infilename, option_values.test_only? 0 : outfilename, option_values.analyze, option_values.aopts, decode_options);
	}
	else {
		decode_options.format_options.iff.foreign_metadata = foreign_metadata;

		retval = flac__decode_file(infilename, option_values.test_only? 0 : outfilename, option_values.analyze, option_values.aopts, decode_options);

	}

	if(foreign_metadata)
		flac__foreign_metadata_delete(foreign_metadata);

	if(retval == 0 && strcmp(infilename, "-")) {
		if(option_values.preserve_modtime && strcmp(outfilename, "-"))
			grabbag__file_copy_metadata(infilename, outfilename);
		if(option_values.delete_input && !option_values.test_only && !option_values.analyze)
			flac_unlink(infilename);
	}

	return retval;