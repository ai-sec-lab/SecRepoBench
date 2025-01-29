#ifdef DEBUG_VERBOSE
	printf( "vips_foreign_load_jxl_fill_input: %zd bytes requested\n", 
		INPUT_BUFFER_SIZE - bytes_remaining );
#endif /*DEBUG_VERBOSE*/

	memmove( jxl->input_buffer,
		jxl->input_buffer + jxl->bytes_in_buffer - bytes_remaining,
		bytes_remaining );
	bytes_read = vips_source_read( jxl->source,
		jxl->input_buffer + bytes_remaining,
		INPUT_BUFFER_SIZE - bytes_remaining );