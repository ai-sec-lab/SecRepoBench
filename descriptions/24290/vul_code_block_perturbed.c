i = VIPS_CLIP( 0, nX, QUANT_ELEMENTS - 2 );
		fractionalpart = nX - i;
		cbx = cbrt_table[i] + fractionalpart * (cbrt_table[i + 1] - cbrt_table[i]);

		i = VIPS_CLIP( 0, nY, QUANT_ELEMENTS - 2 );
		fractionalpart = nY - i;
		cby = cbrt_table[i] + fractionalpart * (cbrt_table[i + 1] - cbrt_table[i]);

		i = VIPS_CLIP( 0, nZ, QUANT_ELEMENTS - 2 );