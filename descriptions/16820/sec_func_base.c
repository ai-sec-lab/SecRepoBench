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
			int k;

			for( k = 1; k < 360 && hl[j][k] <= i; k++ ) 
				;

			hI[j][i] = k - 1 + (i - hl[j][k - 1]) / 
				(hl[j][k] - hl[j][k - 1]);
		}
	}
}