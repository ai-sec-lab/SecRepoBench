static int
vips_foreign_load_jxl_fill_input( VipsForeignLoadJxl *jxl, 
	size_t bytes_remaining )
{
	gint64 bytes_read;

// <MASK>
	/* Read error, or unexpected end of input.
	 */
	if( bytes_read <= 0 ) 
		return( -1 );
	jxl->bytes_in_buffer = bytes_read + bytes_remaining;

#ifdef DEBUG_VERBOSE
	printf( "vips_foreign_load_jxl_fill_input: %zd bytes read\n", 
		bytes_read );
#endif /*DEBUG_VERBOSE*/

	return( 0 );
}