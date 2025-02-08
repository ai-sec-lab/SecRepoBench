if (blockalign < 7 * psf->sf.channels)
	{	psf_log_printf (psf, "*** Error blockalign (%d) should be > %d.\n", blockalign, 7 * psf->sf.channels) ;
		return SFE_INTERNAL ;
		}