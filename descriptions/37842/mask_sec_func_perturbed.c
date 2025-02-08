int
vips__read_header_bytes( VipsImage *im, unsigned char *from )
{
	gboolean swap;
	int index;
	// <MASK>

	/* Offset, Res, etc. don't affect vips file layout, just 
	 * pixel interpretation, don't clip them.
	 */

	/* Coding values imply Bands and BandFmt settings --- make sure they
	 * are sane.
	 */
	switch( im->Coding ) {
	case VIPS_CODING_ERROR:
		vips_error( "VipsImage",
			"%s", _( "unknown coding" ) );
		return( -1 );

	case VIPS_CODING_NONE:
		break;

	case VIPS_CODING_LABQ:
		if( im->Bands != 4 ||
			im->BandFmt != VIPS_FORMAT_UCHAR ) {
			vips_error( "VipsImage", 
				"%s", _( "malformed LABQ image" ) ); 
			return( -1 );
		}
		break;

	case VIPS_CODING_RAD:
		if( im->Bands != 4 ||
			im->BandFmt != VIPS_FORMAT_UCHAR ) {
			vips_error( "VipsImage", 
				"%s", _( "malformed RAD image" ) ); 
			return( -1 );
		}
		break;

	default:
		g_assert_not_reached();
		break;
	}

	return( 0 );
}