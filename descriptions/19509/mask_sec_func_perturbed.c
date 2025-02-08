static int process_head_file(struct archive_read* a, struct rar5* rar,
    struct archive_entry* entry, size_t block_flags)
{
	ssize_t extra_data_size = 0;
	size_t data_size = 0;
	size_t filestatusflags = 0;
	size_t file_attr = 0;
	size_t compression_info = 0;
	size_t host_os = 0;
	size_t name_size = 0;
	uint64_t unpacked_size, window_size;
	uint32_t mtime = 0, crc = 0;
	int c_method = 0, c_version = 0;
	char name_utf8_buf[MAX_NAME_IN_BYTES];
	const uint8_t* p;

	archive_entry_clear(entry);

	/* Do not reset file context if we're switching archives. */
	if(!rar->cstate.switch_multivolume) {
		reset_file_context(rar);
	}

	if(block_flags & HFL_EXTRA_DATA) {
		size_t edata_size = 0;
		if(!read_var_sized(a, &edata_size, NULL))
			return ARCHIVE_EOF;

		/* Intentional type cast from unsigned to signed. */
		extra_data_size = (ssize_t) edata_size;
	}

	if(block_flags & HFL_DATA) {
		if(!read_var_sized(a, &data_size, NULL))
			return ARCHIVE_EOF;

		rar->file.bytes_remaining = data_size;
	} else {
		rar->file.bytes_remaining = 0;

		archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
				"no data found in file/service block");
		return ARCHIVE_FATAL;
	}

	enum FILE_FLAGS {
		DIRECTORY = 0x0001, UTIME = 0x0002, CRC32 = 0x0004,
		UNKNOWN_UNPACKED_SIZE = 0x0008,
	};

	enum FILE_ATTRS {
		ATTR_READONLY = 0x1, ATTR_HIDDEN = 0x2, ATTR_SYSTEM = 0x4,
		ATTR_DIRECTORY = 0x10,
	};

	enum COMP_INFO_FLAGS {
		SOLID = 0x0040,
	};

	if(!read_var_sized(a, &filestatusflags, NULL))
		return ARCHIVE_EOF;

	if(!read_var(a, &unpacked_size, NULL))
		return ARCHIVE_EOF;

	if(filestatusflags & UNKNOWN_UNPACKED_SIZE) {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_PROGRAMMER,
		    "Files with unknown unpacked size are not supported");
		return ARCHIVE_FATAL;
	}

	rar->file.dir = (uint8_t) ((filestatusflags & DIRECTORY) > 0);

	if(!read_var_sized(a, &file_attr, NULL))
		return ARCHIVE_EOF;

	if(filestatusflags & UTIME) {
		if(!read_u32(a, &mtime))
			return ARCHIVE_EOF;
	}

	if(filestatusflags & CRC32) {
		if(!read_u32(a, &crc))
			return ARCHIVE_EOF;
	}

	if(!read_var_sized(a, &compression_info, NULL))
		return ARCHIVE_EOF;

	c_method = (int) (compression_info >> 7) & 0x7;
	c_version = (int) (compression_info & 0x3f);

	/* RAR5 seems to limit the dictionary size to 64MB. */
	window_size = (rar->file.dir > 0) ?
		0 :
		g_unpack_window_size << ((compression_info >> 10) & 15);
	rar->cstate.method = c_method;
	rar->cstate.version = c_version + 50;
	rar->file.solid = (compression_info & SOLID) > 0;

	/* Archives which declare solid files without initializing the window
	 * buffer first are invalid. */

	if(rar->file.solid > 0 && rar->cstate.window_buf == NULL) {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
				  "Declared solid file, but no window buffer "
				  "initialized yet.");
		return ARCHIVE_FATAL;
	}

	/* Check if window_size is a sane value. Also, if the file is not
	 * declared as a directory, disallow window_size == 0. */
	if(window_size > (64 * 1024 * 1024) ||
	    (rar->file.dir == 0 && window_size == 0))
	{
		archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
		    "Declared dictionary size is not supported.");
		return ARCHIVE_FATAL;
	}

	if(rar->file.solid > 0) {
		/* Re-check if current window size is the same as previous
		 * window size (for solid files only). */
		if(rar->file.solid_window_size > 0 &&
		    rar->file.solid_window_size != (ssize_t) window_size)
		{
			archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
			    "Window size for this solid file doesn't match "
			    "the window size used in previous solid file. ");
			return ARCHIVE_FATAL;
		}
	}

	/* If we're currently switching volumes, ignore the new definition of
	 * window_size. */
	// <MASK>
}