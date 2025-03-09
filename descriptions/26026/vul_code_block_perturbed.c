if (chunksize >= sizeof (SF_CART_INFO_16K))
	{	psf_log_printf (psf, "cart : %u too big to be handled\n", chunksize) ;
		psf_binheader_readf (psf, "j", chunksize) ;
		return 0 ;
		}