readcount = psf_fread (ubuf.dbuf, sizeof (double), bufferlen, psf) ;

		if (psf->data_endswap == SF_TRUE)
			endswap_double_array (ubuf.dbuf, readcount) ;

		d2f_array (ubuf.dbuf, readcount, ptr + total) ;
		total += readcount ;