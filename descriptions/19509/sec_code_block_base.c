if(rar->cstate.switch_multivolume == 0) {
		/* Values up to 64M should fit into ssize_t on every
		 * architecture. */
		rar->cstate.window_size = (ssize_t) window_size;
	}

	if(rar->file.solid > 0 && rar->file.solid_window_size == 0) {
		/* Solid files have to have the same window_size across
		   whole archive. Remember the window_size parameter
		   for first solid file found. */
		rar->file.solid_window_size = rar->cstate.window_size;
	}

	init_window_mask(rar);

	rar->file.service = 0;

	if(!read_var_sized(a, &host_os, NULL))
		return ARCHIVE_EOF;

	enum HOST_OS {
		HOST_WINDOWS = 0,
		HOST_UNIX = 1,
	};

	if(host_os == HOST_WINDOWS) {
		/* Host OS is Windows */

		__LA_MODE_T mode;

		if(file_attr & ATTR_DIRECTORY) {
			if (file_attr & ATTR_READONLY) {
				mode = 0555 | AE_IFDIR;
			} else {
				mode = 0755 | AE_IFDIR;
			}
		} else {
			if (file_attr & ATTR_READONLY) {
				mode = 0444 | AE_IFREG;
			} else {
				mode = 0644 | AE_IFREG;
			}
		}

		archive_entry_set_mode(entry, mode);

		if (file_attr & (ATTR_READONLY | ATTR_HIDDEN | ATTR_SYSTEM)) {
			char *fflags_text, *ptr;
			/* allocate for "rdonly,hidden,system," */
			fflags_text = malloc(22 * sizeof(char));
			if (fflags_text != NULL) {
				ptr = fflags_text;
				if (file_attr & ATTR_READONLY) {
					strcpy(ptr, "rdonly,");
					ptr = ptr + 7;
				}
				if (file_attr & ATTR_HIDDEN) {
					strcpy(ptr, "hidden,");
					ptr = ptr + 7;
				}
				if (file_attr & ATTR_SYSTEM) {
					strcpy(ptr, "system,");
					ptr = ptr + 7;
				}
				if (ptr > fflags_text) {
					/* Delete trailing comma */
					*(ptr - 1) = '\0';
					archive_entry_copy_fflags_text(entry,
					    fflags_text);
				}
				free(fflags_text);
			}
		}
	} else if(host_os == HOST_UNIX) {
		/* Host OS is Unix */
		archive_entry_set_mode(entry, (__LA_MODE_T) file_attr);
	} else {
		/* Unknown host OS */
		archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
				"Unsupported Host OS: 0x%x", (int) host_os);

		return ARCHIVE_FATAL;
	}

	if(!read_var_sized(a, &name_size, NULL))
		return ARCHIVE_EOF;

	if(!read_ahead(a, name_size, &p))
		return ARCHIVE_EOF;

	if(name_size > (MAX_NAME_IN_CHARS - 1)) {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
				"Filename is too long");

		return ARCHIVE_FATAL;
	}

	if(name_size == 0) {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
				"No filename specified");

		return ARCHIVE_FATAL;
	}

	memcpy(name_utf8_buf, p, name_size);
	name_utf8_buf[name_size] = 0;
	if(ARCHIVE_OK != consume(a, name_size)) {
		return ARCHIVE_EOF;
	}

	archive_entry_update_pathname_utf8(entry, name_utf8_buf);

	if(extra_data_size > 0) {
		int ret = process_head_file_extra(a, entry, rar,
		    extra_data_size);

		/* Sanity check. */
		if(extra_data_size < 0) {
			archive_set_error(&a->archive, ARCHIVE_ERRNO_PROGRAMMER,
			    "File extra data size is not zero");
			return ARCHIVE_FATAL;
		}

		if(ret != ARCHIVE_OK)
			return ret;
	}

	if((file_flags & UNKNOWN_UNPACKED_SIZE) == 0) {
		rar->file.unpacked_size = (ssize_t) unpacked_size;
		if(rar->file.redir_type == REDIR_TYPE_NONE)
			archive_entry_set_size(entry, unpacked_size);
	}

	if(file_flags & UTIME) {
		archive_entry_set_mtime(entry, (time_t) mtime, 0);
	}

	if(file_flags & CRC32) {
		rar->file.stored_crc32 = crc;
	}

	if(!rar->cstate.switch_multivolume) {
		/* Do not reinitialize unpacking state if we're switching
		 * archives. */
		rar->cstate.block_parsing_finished = 1;
		rar->cstate.all_filters_applied = 1;
		rar->cstate.initialized = 0;
	}

	if(rar->generic.split_before > 0) {
		/* If now we're standing on a header that has a 'split before'
		 * mark, it means we're standing on a 'continuation' file
		 * header. Signal the caller that if it wants to move to
		 * another file, it must call rar5_read_header() function
		 * again. */

		return ARCHIVE_RETRY;
	} else {
		return ARCHIVE_OK;
	}