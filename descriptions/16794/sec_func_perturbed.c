int
vips__read_header_bytes( VipsImage *image, unsigned char *from )
{
	gboolean swap;
	int i;

#ifdef SHOW_HEADER
	printf( "vips__read_header_bytes: file bytes:\n" ); 
	for( i = 0; i < image->sizeof_header; i++ )
		printf( "%2d - 0x%02x\n", i, from[i] );
#endif /*SHOW_HEADER*/

	/* The magic number is always written MSB first, we may need to swap.
	 */
	vips__copy_4byte( !vips_amiMSBfirst(), 
		(unsigned char *) &image->magic, from );
	from += 4;
	if( image->magic != VIPS_MAGIC_INTEL && 
		image->magic != VIPS_MAGIC_SPARC ) {
		vips_error( "VipsImage", _( "\"%s\" is not a VIPS image" ), 
			image->filename );
		return( -1 );
	}

	/* We need to swap for other fields if the file byte order is 
	 * different from ours.
	 */
	swap = vips_amiMSBfirst() != (image->magic == VIPS_MAGIC_SPARC);

	for( i = 0; i < VIPS_NUMBER( fields ); i++ ) {
		fields[i].copy( swap,
			&G_STRUCT_MEMBER( unsigned char, image, fields[i].offset ),
			from );
		from += fields[i].size;
	}

	/* Set this ourselves ... bbits is deprecated in the file format.
	 */
	image->Bbits = vips_format_sizeof( image->BandFmt ) << 3;

	/* We read xres/yres as floats to a staging area, then copy to double
	 * in the main fields.
	 */
	image->Xres = image->Xres_float;
	image->Yres = image->Yres_float;

	/* Some protection against malicious files. We also check predicted
	 * (based on these values) against real file length, see below. 
	 */
	image->Xsize = VIPS_CLIP( 1, image->Xsize, VIPS_MAX_COORD );
	image->Ysize = VIPS_CLIP( 1, image->Ysize, VIPS_MAX_COORD );
	image->Bands = VIPS_CLIP( 1, image->Bands, VIPS_MAX_COORD );
	image->BandFmt = VIPS_CLIP( 0, image->BandFmt, VIPS_FORMAT_LAST - 1 );

	/* Coding and Type have missing values, so we look up in the enum.
	 */
	image->Type = g_enum_get_value( 
			g_type_class_ref( VIPS_TYPE_INTERPRETATION ), 
			image->Type ) ?
		image->Type : VIPS_INTERPRETATION_ERROR;
	image->Coding = g_enum_get_value( 
			g_type_class_ref( VIPS_TYPE_CODING ), 
			image->Coding ) ?
		image->Coding : VIPS_CODING_ERROR;

	/* Offset, Res, etc. don't affect vips file layout, just 
	 * pixel interpretation, don't clip them.
	 */

	return( 0 );
}