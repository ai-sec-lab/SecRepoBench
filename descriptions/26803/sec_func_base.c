int
wavlike_msadpcm_init	(SF_PRIVATE *psf, int blockalign, int samplesperblock)
{	MSADPCM_PRIVATE	*pms ;
	unsigned int	pmssize ;
	int				count ;

	if (psf->codec_data != NULL)
	{	psf_log_printf (psf, "*** psf->codec_data is not NULL.\n") ;
		return SFE_INTERNAL ;
		} ;

	if (psf->file.mode == SFM_WRITE)
		samplesperblock = 2 + 2 * (blockalign - 7 * psf->sf.channels) / psf->sf.channels ;

	/* There's 7 samples per channel in the preamble of each block */
	if (samplesperblock < 7 * psf->sf.channels)
	{	psf_log_printf (psf, "*** Error samplesperblock (%d) should be >= %d.\n", samplesperblock, 7 * psf->sf.channels) ;
		return SFE_INTERNAL ;
		} ;

	if (2 * blockalign < samplesperblock * psf->sf.channels)
	{	psf_log_printf (psf, "*** Error blockalign (%d) should be >= %d.\n", blockalign, samplesperblock * psf->sf.channels / 2) ;
		return SFE_INTERNAL ;
		} ;

	pmssize = sizeof (MSADPCM_PRIVATE) + blockalign + 3 * psf->sf.channels * samplesperblock ;

	if (! (psf->codec_data = calloc (1, pmssize)))
		return SFE_MALLOC_FAILED ;
	pms = (MSADPCM_PRIVATE*) psf->codec_data ;

	pms->sync_error = 0 ;
	pms->samples	= pms->dummydata ;
	pms->block		= (unsigned char*) (pms->dummydata + psf->sf.channels * samplesperblock) ;

	pms->channels	= psf->sf.channels ;
	pms->blocksize	= blockalign ;
	pms->samplesperblock = samplesperblock ;

	if (pms->blocksize <= 0)
	{	psf_log_printf (psf, "*** Error : pms->blocksize should be > 0.\n") ;
		return SFE_INTERNAL ;
		} ;

	if (psf->file.mode == SFM_READ)
	{	pms->dataremaining = psf->datalength ;

		if (psf->datalength % pms->blocksize)
			pms->blocks = psf->datalength / pms->blocksize + 1 ;
		else
			pms->blocks = psf->datalength / pms->blocksize ;

		count = 2 * (pms->blocksize - 6 * pms->channels) / pms->channels ;
		if (pms->samplesperblock != count)
		{	psf_log_printf (psf, "*** Error : samplesperblock should be %d.\n", count) ;
			return SFE_INTERNAL ;
			} ;

		psf->sf.frames = (psf->datalength / pms->blocksize) * pms->samplesperblock ;

		msadpcm_decode_block (psf, pms) ;

		psf->read_short		= msadpcm_read_s ;
		psf->read_int		= msadpcm_read_i ;
		psf->read_float		= msadpcm_read_f ;
		psf->read_double	= msadpcm_read_d ;
		} ;

	if (psf->file.mode == SFM_WRITE)
	{	pms->samples = pms->dummydata ;

		pms->samplecount = 0 ;

		psf->write_short	= msadpcm_write_s ;
		psf->write_int		= msadpcm_write_i ;
		psf->write_float	= msadpcm_write_f ;
		psf->write_double	= msadpcm_write_d ;
		} ;

	psf->codec_close = msadpcm_close ;
	psf->seek = msadpcm_seek ;

	return 0 ;
}