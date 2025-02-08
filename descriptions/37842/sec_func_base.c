int
vips__read_header_bytes( VipsImage *im, unsigned char *from )
{
	gboolean swap;
	int i;
	GEnumValue *value;

#ifdef SHOW_HEADER
	printf( "vips__read_header_bytes: file bytes:\n" ); 
	for( i = 0; i < im->sizeof_header; i++ )
		printf( "%2d - 0x%02x\n", i, from[i] );
#endif /*SHOW_HEADER*/

	/* The magic number is always written MSB first, we may need to swap.
	 */
	vips__copy_4byte( !vips_amiMSBfirst(), 
		(unsigned char *) &im->magic, from );
	from += 4;
	if( im->magic != VIPS_MAGIC_INTEL && 
		im->magic != VIPS_MAGIC_SPARC ) {
		vips_error( "VipsImage", 
			_( "\"%s\" is not a VIPS image" ), im->filename );
		return( -1 );
	}

	/* We need to swap for other fields if the file byte order is 
	 * different from ours.
	 */
	swap = vips_amiMSBfirst() != vips_image_isMSBfirst( im );

	for( i = 0; i < VIPS_NUMBER( fields ); i++ ) {
		fields[i].copy( swap,
			&G_STRUCT_MEMBER( unsigned char, im, fields[i].offset ),
			from );
		from += fields[i].size;
	}

	/* Set this ourselves ... bbits is deprecated in the file format.
	 */
	im->Bbits = vips_format_sizeof( im->BandFmt ) << 3;

	/* We read xres/yres as floats to a staging area, then copy to double
	 * in the main fields.
	 */
	im->Xres = im->Xres_float;
	im->Yres = im->Yres_float;

	/* Some protection against malicious files. We also check predicted
	 * (based on these values) against real file length, see below. 
	 */
	im->Xsize = VIPS_CLIP( 1, im->Xsize, VIPS_MAX_COORD );
	im->Ysize = VIPS_CLIP( 1, im->Ysize, VIPS_MAX_COORD );
	im->Bands = VIPS_CLIP( 1, im->Bands, VIPS_MAX_COORD );
	im->BandFmt = VIPS_CLIP( 0, im->BandFmt, VIPS_FORMAT_LAST - 1 );

	/* Coding and Type have missing values, so we look up in the enum.
	 */
	value = g_enum_get_value( g_type_class_ref( VIPS_TYPE_INTERPRETATION ),
		im->Type );
	if( !value ||
		strcmp( value->value_nick, "last" ) == 0 )
		im->Type = VIPS_INTERPRETATION_ERROR;
	value = g_enum_get_value( g_type_class_ref( VIPS_TYPE_CODING ),
		im->Coding );
	if( !value ||
		strcmp( value->value_nick, "last" ) == 0 )
		im->Coding = VIPS_CODING_ERROR;

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