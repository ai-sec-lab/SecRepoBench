/* Read magic1,magic2,lzma_params from the ZIPX stream. */
	if(zip->entry_bytes_remaining < 9 || (lzma_stream_pointer = __archive_read_ahead(a, 9, NULL)) == NULL) {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
		    "Truncated lzma data");
		return (ARCHIVE_FATAL);
	}

	if(lzma_stream_pointer[2] != 0x05 || lzma_stream_pointer[3] != 0x00) {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
		    "Invalid lzma data");
		return (ARCHIVE_FATAL);
	}

	/* Prepare an lzma alone header: copy the lzma_params blob into
	 * a proper place into the lzma alone header. */