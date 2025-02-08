static int
PackBitsDecode(TIFF* tif, uint8* op, tmsize_t occ, uint16 s)
{
	static const char module[] = "PackBitsDecode";
	char *bp;
	tmsize_t cc;
	long n;
	int byteValue;

	(void) s;
	bp = (char*) tif->tif_rawcp;
	cc = tif->tif_rawcc;
	while (cc > 0 && occ > 0) {
		n = (long) *bp++;
		cc--;
		/*
		 * Watch out for compilers that
		 * don't sign extend chars...
		 */
		if (n >= 128)
			n -= 256;
		if (n < 0) {		/* replicate next byte -n+1 times */
			if (n == -128)	/* nop */
				continue;
			n = -n + 1;
			if( occ < (tmsize_t)n )
			{
				TIFFWarningExt(tif->tif_clientdata, module,
				    "Discarding %lu bytes to avoid buffer overrun",
				    (unsigned long) ((tmsize_t)n - occ));
				n = (long)occ;
			}
			if( cc == 0 )
			{
				TIFFWarningExt(tif->tif_clientdata, module,
					       "Terminating PackBitsDecode due to lack of data.");
				break;
			}
			occ -= n;
			byteValue = *bp++;
			cc--;
			while (n-- > 0)
				*op++ = (uint8) byteValue;
		} else {		/* copy next n+1 bytes literally */
			if (occ < (tmsize_t)(n + 1))
			{
				TIFFWarningExt(tif->tif_clientdata, module,
				    "Discarding %lu bytes to avoid buffer overrun",
				    (unsigned long) ((tmsize_t)n - occ + 1));
				n = (long)occ - 1;
			}
			if (cc < (tmsize_t) (n+1)) 
			{
				TIFFWarningExt(tif->tif_clientdata, module,
					       "Terminating PackBitsDecode due to lack of data.");
				break;
			}
			_TIFFmemcpy(op, bp, ++n);
			op += n; occ -= n;
			bp += n; cc -= n;
		}
	}
	tif->tif_rawcp = (uint8*) bp;
	tif->tif_rawcc = cc;
	if (occ > 0) {
		TIFFErrorExt(tif->tif_clientdata, module,
		    "Not enough data for scanline %lu",
		    (unsigned long) tif->tif_row);
		return (0);
	}
	return (1);
}