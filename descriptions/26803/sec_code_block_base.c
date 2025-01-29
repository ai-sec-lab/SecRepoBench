/* There's 7 samples per channel in the preamble of each block */
	if (samplesperblock < 7 * psf->sf.channels)
	{	psf_log_printf (psf, "*** Error samplesperblock (%d) should be >= %d.\n", samplesperblock, 7 * psf->sf.channels) ;
		return SFE_INTERNAL ;
		} ;

	if (2 * blockalign < samplesperblock * psf->sf.channels)
	{	psf_log_printf (psf, "*** Error blockalign (%d) should be >= %d.\n", blockalign, samplesperblock * psf->sf.channels / 2) ;
		return SFE_INTERNAL ;
		}