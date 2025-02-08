static sf_count_t
host_read_d2f	(SF_PRIVATE *psf, float *ptr, sf_count_t len)
{	BUF_UNION	ubuf ;
	int			bufferlen, readcount ;
	sf_count_t	total = 0 ;

	bufferlen = ARRAY_LEN (ubuf.dbuf) ;

	while (len > 0)
	{	if (len < bufferlen)
			bufferlen = (int) len ;
		readcount = psf_fread (ubuf.dbuf, sizeof (double), bufferlen, psf) ;

		if (psf->data_endswap == SF_TRUE)
			endswap_double_array (ubuf.dbuf, readcount) ;

		d2f_array (ubuf.dbuf, readcount, ptr + total) ;
		total += readcount ;
		len -= readcount ;
		if (readcount < bufferlen)
			break ;
		} ;

	return total ;
}