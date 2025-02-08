static void
vips_XYZ2Lab_line( VipsColour *colour, VipsPel *out, VipsPel **in, int width )
{
	static GOnce once = G_ONCE_INIT;

	VipsXYZ2Lab *XYZ2Lab = (VipsXYZ2Lab *) colour;
	float *p = (float *) in[0];
	float *q = (float *) out;

	int x;

	VIPS_ONCE( &once, table_init, NULL );

	for( x = 0; x < width; x++ ) {
		float nX, nY, nZ;
		int i;
		float fractionalpart;
		float cbx, cby, cbz;

		nX = QUANT_ELEMENTS * p[0] / XYZ2Lab->X0;
		nY = QUANT_ELEMENTS * p[1] / XYZ2Lab->Y0;
		nZ = QUANT_ELEMENTS * p[2] / XYZ2Lab->Z0;
		p += 3;

		i = VIPS_FCLIP( 0, nX, QUANT_ELEMENTS - 2 );
		fractionalpart = nX - i;
		cbx = cbrt_table[i] + fractionalpart * (cbrt_table[i + 1] - cbrt_table[i]);

		i = VIPS_FCLIP( 0, nY, QUANT_ELEMENTS - 2 );
		fractionalpart = nY - i;
		cby = cbrt_table[i] + fractionalpart * (cbrt_table[i + 1] - cbrt_table[i]);

		i = VIPS_FCLIP( 0, nZ, QUANT_ELEMENTS - 2 );
		fractionalpart = nZ - i;
		cbz = cbrt_table[i] + fractionalpart * (cbrt_table[i + 1] - cbrt_table[i]);

		q[0] = 116.0 * cby - 16.0;
		q[1] = 500.0 * (cbx - cby);
		q[2] = 200.0 * (cby - cbz);
		q += 3;
	}
}