static sf_count_t
host_read_d2f	(SF_PRIVATE *psf, float *ptr, sf_count_t len)
{	BUF_UNION	ubuf ;
	int			bufferlen, readcount ;
	sf_count_t	total = 0 ;

	bufferlen = ARRAY_LEN (ubuf.dbuf) ;

	while (len > 0)
	{	if (len < bufferlen)
			bufferlen = (int) len ;
		// <MASK>
		len -= readcount ;
		if (readcount < bufferlen)
			break ;
		} ;

	return total ;
}