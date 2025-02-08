static int
vips_foreign_load_jxl_fill_input( VipsForeignLoadJxl *jxlloader, 
	size_t bytes_remaining )
{
	gint64 bytes_read;

#ifdef DEBUG_VERBOSE
	printf( "vips_foreign_load_jxl_fill_input: %zd bytes requested\n", 
		INPUT_BUFFER_SIZE - bytes_remaining );
#endif /*DEBUG_VERBOSE*/

	memmove( jxlloader->input_buffer,
		jxlloader->input_buffer + jxlloader->bytes_in_buffer - bytes_remaining,
		bytes_remaining );
	bytes_read = vips_source_read( jxlloader->source,
		jxlloader->input_buffer + bytes_remaining,
		INPUT_BUFFER_SIZE - bytes_remaining );
	/* Read error, or unexpected end of input.
	 */
	if( bytes_read <= 0 ) 
		return( -1 );
	jxlloader->bytes_in_buffer = bytes_read + bytes_remaining;

#ifdef DEBUG_VERBOSE
	printf( "vips_foreign_load_jxl_fill_input: %zd bytes read\n", 
		bytes_read );
#endif /*DEBUG_VERBOSE*/

	return( 0 );
}