static int
vips_foreign_load_matrix_load( VipsForeignLoad *load )
{
	VipsForeignLoadMatrix *matrix = (VipsForeignLoadMatrix *) load;
	VipsObjectClass *class = VIPS_OBJECT_GET_CLASS( load );

	int x, row;

	vips_image_pipelinev( load->real, VIPS_DEMAND_STYLE_THINSTRIP, NULL );
	vips_image_init_fields( load->real,
		load->out->Xsize, load->out->Ysize, 1, 
		VIPS_FORMAT_DOUBLE, 
		VIPS_CODING_NONE, VIPS_INTERPRETATION_B_W, 1.0, 1.0 );

	for( row = 0; row < load->real->Ysize; row++ ) {
		char *line;
		char *p, *q;

		line = vips_sbuf_get_line_copy( matrix->sbuf );

		for( x = 0, p = line; 
			(q = vips_break_token( p, " \t" )) &&
				x < load->out->Xsize;
			x++, p = q )
			if( vips_strtod( p, &matrix->linebuf[x] ) ) {
				vips_error( class->nickname, 
					_( "bad number \"%s\"" ), p );
				g_free( line );
				return( -1 );
			}

		g_free( line );

		if( x != load->out->Xsize ) {
			vips_error( class->nickname, 
				_( "line %d too short" ), row );
			return( -1 );
		}

		if( vips_image_write_line( load->real, row, 
			(VipsPel *) matrix->linebuf ) )
			return( -1 );
	}

	return( 0 );
}