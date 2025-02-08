/* There's 7 samples per channel in the preamble of each block */
	if (samplesperblock < 7 * audiofileprivate->sf.channels)
	{	psf_log_printf (audiofileprivate, "*** Error samplesperblock (%d) should be >= %d.\n", samplesperblock, 7 * audiofileprivate->sf.channels) ;
		return SFE_INTERNAL ;
		} ;

	if (2 * blockalign < samplesperblock * audiofileprivate->sf.channels)
	{	psf_log_printf (audiofileprivate, "*** Error blockalign (%d) should be >= %d.\n", blockalign, samplesperblock * audiofileprivate->sf.channels / 2) ;
		return SFE_INTERNAL ;
		}