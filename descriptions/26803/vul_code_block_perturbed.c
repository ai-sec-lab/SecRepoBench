if (blockalign < 7 * audiofileprivate->sf.channels)
	{	psf_log_printf (audiofileprivate, "*** Error blockalign (%d) should be > %d.\n", blockalign, 7 * audiofileprivate->sf.channels) ;
		return SFE_INTERNAL ;
		}