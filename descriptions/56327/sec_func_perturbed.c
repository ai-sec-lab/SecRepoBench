FLAC__bool EncoderSession_construct(EncoderSession *e, encode_options_t encodingoptions, FLAC__off_t infilesize, FILE *infile, const char *infilename, const char *outfilename, const FLAC__byte *lookahead, uint32_t lookahead_length)
{
	uint32_t i;
	FLAC__uint32 test = 1;

	/*
	 * initialize globals
	 */

	is_big_endian_host_ = (*((FLAC__byte*)(&test)))? false : true;

	for(i = 0; i < FLAC__MAX_CHANNELS; i++)
		input_[i] = &(in_[i][0]);


	/*
	 * initialize instance
	 */

#if FLAC__HAS_OGG
	e->use_ogg = encodingoptions.use_ogg;
#endif
	e->verify = encodingoptions.verify;
	e->treat_warnings_as_errors = encodingoptions.treat_warnings_as_errors;
	e->continue_through_decode_errors = encodingoptions.continue_through_decode_errors;

	e->is_stdout = (0 == strcmp(outfilename, "-"));
	e->outputfile_opened = false;

	e->inbasefilename = grabbag__file_get_basename(infilename);
	e->infilename = infilename;
	e->outfilename = outfilename;

	e->total_samples_to_encode = 0;
	e->unencoded_size = 0;
	e->bytes_written = 0;
	e->samples_written = 0;
#if 0 /* in case time.h with clock() isn't available for some reason */
	e->stats_frames_interval = 0;
	e->old_frames_written = 0;
#else
	e->old_clock_t = 0;
#endif
	e->compression_ratio = 0.0;

	memset(&e->info, 0, sizeof(e->info));

	e->format = encodingoptions.format;

	switch(encodingoptions.format) {
		case FORMAT_RAW:
			break;
		case FORMAT_WAVE:
		case FORMAT_WAVE64:
		case FORMAT_RF64:
		case FORMAT_AIFF:
		case FORMAT_AIFF_C:
			e->fmt.iff.data_bytes = 0;
			break;
		case FORMAT_FLAC:
		case FORMAT_OGGFLAC:
			e->fmt.flac.decoder = 0;
			e->fmt.flac.client_data.filesize = infilesize;
			e->fmt.flac.client_data.lookahead = lookahead;
			e->fmt.flac.client_data.lookahead_length = lookahead_length;
			e->fmt.flac.client_data.num_metadata_blocks = 0;
			e->fmt.flac.client_data.samples_left_to_process = 0;
			e->fmt.flac.client_data.fatal_error = false;
			break;
		default:
			FLAC__ASSERT(0);
			/* double protection */
			return false;
	}

	e->encoder = 0;

	e->fin = infile;
	e->seek_table_template = 0;

	if(0 == (e->seek_table_template = FLAC__metadata_object_new(FLAC__METADATA_TYPE_SEEKTABLE))) {
		flac__utils_printf(stderr, 1, "%s: ERROR allocating memory for seek table\n", e->inbasefilename);
		return false;
	}

	e->encoder = FLAC__stream_encoder_new();
	if(0 == e->encoder) {
		flac__utils_printf(stderr, 1, "%s: ERROR creating the encoder instance\n", e->inbasefilename);
		EncoderSession_destroy(e);
		return false;
	}

	return true;
}