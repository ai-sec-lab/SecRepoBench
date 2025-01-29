const size_t SMALLEST_RAR5_BLOCK_SIZE = 3;

	struct rar5* rar = get_context(a);
	uint32_t hdr_crc, computed_crc;
	size_t raw_hdr_size = 0, hdr_size_len, hdr_size;
	size_t header_id = 0;
	size_t header_flags = 0;
	const uint8_t* p;
	int ret;

	enum HEADER_TYPE {
		HEAD_MARK    = 0x00, HEAD_MAIN  = 0x01, HEAD_FILE   = 0x02,
		HEAD_SERVICE = 0x03, HEAD_CRYPT = 0x04, HEAD_ENDARC = 0x05,
		HEAD_UNKNOWN = 0xff,
	};

	/* Skip any unprocessed data for this file. */
	ret = skip_unprocessed_bytes(a);
	if(ret != ARCHIVE_OK)
		return ret;

	/* Read the expected CRC32 checksum. */
	if(!read_u32(a, &hdr_crc)) {
		return ARCHIVE_EOF;
	}

	/* Read header size. */
	if(!read_var_sized(a, &raw_hdr_size, &hdr_size_len)) {
		return ARCHIVE_EOF;
	}

	hdr_size = raw_hdr_size + hdr_size_len;

	/* Sanity check, maximum header size for RAR5 is 2MB. */
	if(hdr_size > (2 * 1024 * 1024)) {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
		    "Base block header is too large");

		return ARCHIVE_FATAL;
	}

	/* Additional sanity checks to weed out invalid files. */
	if(raw_hdr_size == 0 || hdr_size_len == 0 ||
		hdr_size < SMALLEST_RAR5_BLOCK_SIZE)
	{
		archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
		    "Too small block encountered (%ld bytes)",
		    raw_hdr_size);

		return ARCHIVE_FATAL;
	}