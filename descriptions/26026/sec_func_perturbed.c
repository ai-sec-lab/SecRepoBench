int
wavlike_read_cart_chunk (SF_PRIVATE *audio_file_private, uint32_t chunksize)
{	SF_CART_INFO_16K *c ;
	uint32_t bytes = 0 ;
	int k ;

	if (chunksize < WAV_CART_MIN_CHUNK_SIZE)
	{	psf_log_printf (audio_file_private, "cart : %u (should be >= %d)\n", chunksize, WAV_CART_MIN_CHUNK_SIZE) ;
		psf_binheader_readf (audio_file_private, "j", chunksize) ;
		return 0 ;
		} ;
	if (chunksize > WAV_CART_MAX_CHUNK_SIZE)
	{	psf_log_printf (audio_file_private, "cart : %u (should be < %d)\n", chunksize, WAV_CART_MAX_CHUNK_SIZE) ;
		psf_binheader_readf (audio_file_private, "j", chunksize) ;
		return 0 ;
		} ;

	/*
	**	SF_CART_INFO_16K has an extra field 'tag_text_size' that isn't part
	**	of the chunk, so don't include it in the size check.
	*/
	if (chunksize >= sizeof (SF_CART_INFO_16K) - 4)
	{	psf_log_printf (audio_file_private, "cart : %u too big to be handled\n", chunksize) ;
		psf_binheader_readf (audio_file_private, "j", chunksize) ;
		return 0 ;
		} ;

	psf_log_printf (audio_file_private, "cart : %u\n", chunksize) ;

	if (audio_file_private->cart_16k)
	{	psf_log_printf (audio_file_private, "  Found more than one cart chunk, using last one.\n") ;
		free (audio_file_private->cart_16k) ;
		audio_file_private->cart_16k = NULL ;
		} ;

	if ((audio_file_private->cart_16k = cart_var_alloc ()) == NULL)
	{	audio_file_private->error = SFE_MALLOC_FAILED ;
		return audio_file_private->error ;
		} ;

	c = audio_file_private->cart_16k ;
	bytes += psf_binheader_readf (audio_file_private, "b", c->version, sizeof (c->version)) ;
	bytes += psf_binheader_readf (audio_file_private, "b", c->title, sizeof (c->title)) ;
	bytes += psf_binheader_readf (audio_file_private, "b", c->artist, sizeof (c->artist)) ;
	bytes += psf_binheader_readf (audio_file_private, "b", c->cut_id, sizeof (c->cut_id)) ;
	bytes += psf_binheader_readf (audio_file_private, "b", c->client_id, sizeof (c->client_id)) ;
	bytes += psf_binheader_readf (audio_file_private, "b", c->category, sizeof (c->category)) ;
	bytes += psf_binheader_readf (audio_file_private, "b", c->classification, sizeof (c->classification)) ;
	bytes += psf_binheader_readf (audio_file_private, "b", c->out_cue, sizeof (c->out_cue)) ;
	bytes += psf_binheader_readf (audio_file_private, "b", c->start_date, sizeof (c->start_date)) ;
	bytes += psf_binheader_readf (audio_file_private, "b", c->start_time, sizeof (c->start_time)) ;
	bytes += psf_binheader_readf (audio_file_private, "b", c->end_date, sizeof (c->end_date)) ;
	bytes += psf_binheader_readf (audio_file_private, "b", c->end_time, sizeof (c->end_time)) ;
	bytes += psf_binheader_readf (audio_file_private, "b", c->producer_app_id, sizeof (c->producer_app_id)) ;
	bytes += psf_binheader_readf (audio_file_private, "b", c->producer_app_version, sizeof (c->producer_app_version)) ;
	bytes += psf_binheader_readf (audio_file_private, "b", c->user_def, sizeof (c->user_def)) ;
	bytes += psf_binheader_readf (audio_file_private, "e4", &c->level_reference, sizeof (c->level_reference)) ;

	for (k = 0 ; k < ARRAY_LEN (c->post_timers) ; k++)
		bytes += psf_binheader_readf (audio_file_private, "b4", &c->post_timers [k].usage, make_size_t (4), &c->post_timers [k].value) ;

	bytes += psf_binheader_readf (audio_file_private, "b", c->reserved, sizeof (c->reserved)) ;
	bytes += psf_binheader_readf (audio_file_private, "b", c->url, sizeof (c->url)) ;

	if (chunksize > WAV_CART_MIN_CHUNK_SIZE)
	{	/* File has tag text. */
		c->tag_text_size = chunksize - WAV_CART_MIN_CHUNK_SIZE ;
		bytes += psf_binheader_readf (audio_file_private, "b", c->tag_text, make_size_t (c->tag_text_size)) ;
		} ;

	return 0 ;
}