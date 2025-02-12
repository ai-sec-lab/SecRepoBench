/*
	**	SF_CART_INFO_16K has an extra field 'tag_text_size' that isn't part
	**	of the chunk, so don't include it in the size check.
	*/
	if (chunksize >= sizeof (SF_CART_INFO_16K) - 4)
	{	psf_log_printf (psf, "cart : %u too big to be handled\n", chunksize) ;
		psf_binheader_readf (psf, "j", chunksize) ;
		return 0 ;
		}