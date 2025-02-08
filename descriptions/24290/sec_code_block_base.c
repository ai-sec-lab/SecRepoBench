i = VIPS_FCLIP( 0, nX, QUANT_ELEMENTS - 2 );
		f = nX - i;
		cbx = cbrt_table[i] + f * (cbrt_table[i + 1] - cbrt_table[i]);

		i = VIPS_FCLIP( 0, nY, QUANT_ELEMENTS - 2 );
		f = nY - i;
		cby = cbrt_table[i] + f * (cbrt_table[i + 1] - cbrt_table[i]);

		i = VIPS_FCLIP( 0, nZ, QUANT_ELEMENTS - 2 );