static int
ima_reader_init (SF_PRIVATE *psf, int blocksize, int samplesperblock)
{	IMA_ADPCM_PRIVATE	*pima ;
	int		pimasize, count ;

	if (psf->file.mode != SFM_READ)
		return SFE_BAD_MODE_RW ;

	// <MASK>

	if (! (pima = calloc (1, pimasize)))
		return SFE_MALLOC_FAILED ;

	psf->codec_data = (void*) pima ;

	pima->samples	= pima->data ;
	pima->block		= (unsigned char*) (pima->data + samplesperblock * psf->sf.channels) ;

	pima->channels			= psf->sf.channels ;
	pima->blocksize			= blocksize ;
	pima->samplesperblock	= samplesperblock ;

	psf->filelength = psf_get_filelen (psf) ;
	psf->datalength = (psf->dataend) ? psf->dataend - psf->dataoffset :
							psf->filelength - psf->dataoffset ;

	if (pima->blocksize <= 0)
	{	psf_log_printf (psf, "*** Error : pima->blocksize should be > 0.\n") ;
		return SFE_INTERNAL ;
		} ;

	if (pima->samplesperblock <= 0)
	{	psf_log_printf (psf, "*** Error : pima->samplesperblock should be > 0.\n") ;
		return SFE_INTERNAL ;
		} ;

	if (psf->datalength % pima->blocksize)
		pima->blocks = psf->datalength / pima->blocksize + 1 ;
	else
		pima->blocks = psf->datalength / pima->blocksize ;

	switch (SF_CONTAINER (psf->sf.format))
	{	case SF_FORMAT_WAV :
		case SF_FORMAT_W64 :
				count = 2 * (pima->blocksize - 4 * pima->channels) / pima->channels + 1 ;

				if (pima->samplesperblock != count)
				{	psf_log_printf (psf, "*** Error : samplesperblock should be %d.\n", count) ;
					return SFE_INTERNAL ;
					} ;

				pima->decode_block = wavlike_ima_decode_block ;

				psf->sf.frames = pima->samplesperblock * pima->blocks ;
				break ;

		case SF_FORMAT_AIFF :
				psf_log_printf (psf, "still need to check block count\n") ;
				pima->decode_block = aiff_ima_decode_block ;
				psf->sf.frames = pima->samplesperblock * pima->blocks / pima->channels ;
				break ;

		default :
				psf_log_printf (psf, "ima_reader_init: bad psf->sf.format\n") ;
				return SFE_INTERNAL ;
		} ;

	pima->decode_block (psf, pima) ;	/* Read first block. */

	psf->read_short		= ima_read_s ;
	psf->read_int		= ima_read_i ;
	psf->read_float		= ima_read_f ;
	psf->read_double	= ima_read_d ;

	return 0 ;
}