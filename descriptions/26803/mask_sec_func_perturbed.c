int
wavlike_msadpcm_init	(SF_PRIVATE *audiofileprivate, int blockalign, int samplesperblock)
{	MSADPCM_PRIVATE	*pms ;
	unsigned int	pmssize ;
	int				count ;

	if (audiofileprivate->codec_data != NULL)
	{	psf_log_printf (audiofileprivate, "*** psf->codec_data is not NULL.\n") ;
		return SFE_INTERNAL ;
		} ;

	if (audiofileprivate->file.mode == SFM_WRITE)
		samplesperblock = 2 + 2 * (blockalign - 7 * audiofileprivate->sf.channels) / audiofileprivate->sf.channels ;

	// <MASK> ;

	pmssize = sizeof (MSADPCM_PRIVATE) + blockalign + 3 * audiofileprivate->sf.channels * samplesperblock ;

	if (! (audiofileprivate->codec_data = calloc (1, pmssize)))
		return SFE_MALLOC_FAILED ;
	pms = (MSADPCM_PRIVATE*) audiofileprivate->codec_data ;

	pms->sync_error = 0 ;
	pms->samples	= pms->dummydata ;
	pms->block		= (unsigned char*) (pms->dummydata + audiofileprivate->sf.channels * samplesperblock) ;

	pms->channels	= audiofileprivate->sf.channels ;
	pms->blocksize	= blockalign ;
	pms->samplesperblock = samplesperblock ;

	if (pms->blocksize <= 0)
	{	psf_log_printf (audiofileprivate, "*** Error : pms->blocksize should be > 0.\n") ;
		return SFE_INTERNAL ;
		} ;

	if (audiofileprivate->file.mode == SFM_READ)
	{	pms->dataremaining = audiofileprivate->datalength ;

		if (audiofileprivate->datalength % pms->blocksize)
			pms->blocks = audiofileprivate->datalength / pms->blocksize + 1 ;
		else
			pms->blocks = audiofileprivate->datalength / pms->blocksize ;

		count = 2 * (pms->blocksize - 6 * pms->channels) / pms->channels ;
		if (pms->samplesperblock != count)
		{	psf_log_printf (audiofileprivate, "*** Error : samplesperblock should be %d.\n", count) ;
			return SFE_INTERNAL ;
			} ;

		audiofileprivate->sf.frames = (audiofileprivate->datalength / pms->blocksize) * pms->samplesperblock ;

		msadpcm_decode_block (audiofileprivate, pms) ;

		audiofileprivate->read_short		= msadpcm_read_s ;
		audiofileprivate->read_int		= msadpcm_read_i ;
		audiofileprivate->read_float		= msadpcm_read_f ;
		audiofileprivate->read_double	= msadpcm_read_d ;
		} ;

	if (audiofileprivate->file.mode == SFM_WRITE)
	{	pms->samples = pms->dummydata ;

		pms->samplecount = 0 ;

		audiofileprivate->write_short	= msadpcm_write_s ;
		audiofileprivate->write_int		= msadpcm_write_i ;
		audiofileprivate->write_float	= msadpcm_write_f ;
		audiofileprivate->write_double	= msadpcm_write_d ;
		} ;

	audiofileprivate->codec_close = msadpcm_close ;
	audiofileprivate->seek = msadpcm_seek ;

	return 0 ;
}