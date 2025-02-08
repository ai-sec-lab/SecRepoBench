static void
make_hI( void )
{
	int i, j;
	float hl[101][361];

	for( i = 0; i < 361; i++ ) 
		for( j = 0; j < 101; j++ ) 
			hl[j][i] = vips_col_Ch2hcmc( j * 2.0, i );

	for( j = 0; j < 101; j++ ) {
		for( i = 0; i < 361; i++ ) {
			int cindex;

			for( cindex = 1; cindex < 360 && hl[j][cindex] <= i; cindex++ ) 
				;

			hI[j][i] = cindex - 1 + (i - hl[j][cindex - 1]) / 
				(hl[j][cindex] - hl[j][cindex - 1]);
		}
	}
}