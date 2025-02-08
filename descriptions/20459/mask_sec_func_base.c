static int process_base_block(struct archive_read* a,
    struct archive_entry* entry)
{
	// <MASK>

	/* Read the whole header data into memory, maximum memory use here is
	 * 2MB. */
	if(!read_ahead(a, hdr_size, &p)) {
		return ARCHIVE_EOF;
	}

	/* Verify the CRC32 of the header data. */
	computed_crc = (uint32_t) crc32(0, p, (int) hdr_size);
	if(computed_crc != hdr_crc) {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
		    "Header CRC error");

		return ARCHIVE_FATAL;
	}

	/* If the checksum is OK, we proceed with parsing. */
	if(ARCHIVE_OK != consume(a, hdr_size_len)) {
		return ARCHIVE_EOF;
	}

	if(!read_var_sized(a, &header_id, NULL))
		return ARCHIVE_EOF;

	if(!read_var_sized(a, &header_flags, NULL))
		return ARCHIVE_EOF;

	rar->generic.split_after = (header_flags & HFL_SPLIT_AFTER) > 0;
	rar->generic.split_before = (header_flags & HFL_SPLIT_BEFORE) > 0;
	rar->generic.size = (int)hdr_size;
	rar->generic.last_header_id = (int)header_id;
	rar->main.endarc = 0;

	/* Those are possible header ids in RARv5. */
	switch(header_id) {
		case HEAD_MAIN:
			ret = process_head_main(a, rar, entry, header_flags);

			/* Main header doesn't have any files in it, so it's
			 * pointless to return to the caller. Retry to next
			 * header, which should be HEAD_FILE/HEAD_SERVICE. */
			if(ret == ARCHIVE_OK)
				return ARCHIVE_RETRY;

			return ret;
		case HEAD_SERVICE:
			ret = process_head_service(a, rar, entry, header_flags);
			return ret;
		case HEAD_FILE:
			ret = process_head_file(a, rar, entry, header_flags);
			return ret;
		case HEAD_CRYPT:
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_FILE_FORMAT,
			    "Encryption is not supported");
			return ARCHIVE_FATAL;
		case HEAD_ENDARC:
			rar->main.endarc = 1;

			/* After encountering an end of file marker, we need
			 * to take into consideration if this archive is
			 * continued in another file (i.e. is it part01.rar:
			 * is there a part02.rar?) */
			if(rar->main.volume) {
				/* In case there is part02.rar, position the
				 * read pointer in a proper place, so we can
				 * resume parsing. */
				ret = scan_for_signature(a);
				if(ret == ARCHIVE_FATAL) {
					return ARCHIVE_EOF;
				} else {
					if(rar->vol.expected_vol_no ==
					    UINT_MAX) {
						archive_set_error(&a->archive,
						    ARCHIVE_ERRNO_FILE_FORMAT,
						    "Header error");
							return ARCHIVE_FATAL;
					}

					rar->vol.expected_vol_no =
					    rar->main.vol_no + 1;
					return ARCHIVE_OK;
				}
			} else {
				return ARCHIVE_EOF;
			}
		case HEAD_MARK:
			return ARCHIVE_EOF;
		default:
			if((header_flags & HFL_SKIP_IF_UNKNOWN) == 0) {
				archive_set_error(&a->archive,
				    ARCHIVE_ERRNO_FILE_FORMAT,
				    "Header type error");
				return ARCHIVE_FATAL;
			} else {
				/* If the block is marked as 'skip if unknown',
				 * do as the flag says: skip the block
				 * instead on failing on it. */
				return ARCHIVE_RETRY;
			}
	}

#if !defined WIN32
	// Not reached.
	archive_set_error(&a->archive, ARCHIVE_ERRNO_PROGRAMMER,
	    "Internal unpacker error");
	return ARCHIVE_FATAL;
#endif
}