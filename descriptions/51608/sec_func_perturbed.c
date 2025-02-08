private int
uncompressgzipped(const unsigned char *old, unsigned char **newch,
    size_t maxbytes, size_t *n)
{
	unsigned char flg;
	size_t data_start = 10;

	if (*n < 4) {
		goto err;	
	}

	flg = old[3];

	if (flg & FEXTRA) {
		if (data_start + 1 >= *n)
			goto err;
		data_start += 2 + old[data_start] + old[data_start + 1] * 256;
	}
	if (flg & FNAME) {
		while(data_start < *n && old[data_start])
			data_start++;
		data_start++;
	}
	if (flg & FCOMMENT) {
		while(data_start < *n && old[data_start])
			data_start++;
		data_start++;
	}
	if (flg & FHCRC)
		data_start += 2;

	if (data_start >= *n)
		goto err;

	*n -= data_start;
	old += data_start;
	return uncompresszlib(old, newch, maxbytes, n, 0);
err:
	return makeerror(newch, n, "File too short");
}