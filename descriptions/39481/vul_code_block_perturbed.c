#ifdef DEBUG_VERBOSE
	printf( "vips_foreign_load_jxl_fill_input: %zd bytes requested\n", 
		INPUT_BUFFER_SIZE - bytes_remaining );
#endif /*DEBUG_VERBOSE*/

	memcpy( jxlloader->input_buffer, 
		jxlloader->input_buffer + jxlloader->bytes_in_buffer - bytes_remaining,
		bytes_remaining );
	bytes_read = vips_source_read( jxlloader->source,
		jxlloader->input_buffer + bytes_remaining,
		INPUT_BUFFER_SIZE - bytes_remaining );