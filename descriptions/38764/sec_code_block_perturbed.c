p = b;
		b += iso9660->logical_block_size;
		step -= iso9660->logical_block_size;
		for (; *p != 0 && p + DR_name_offset < b && p + *p <= b;
			p += *p) {
			struct file_info *child;

			/* N.B.: these special directory identifiers
			 * are 8 bit "values" even on a
			 * Joliet CD with UCS-2 (16bit) encoding.
			 */

			/* Skip '.' entry. */
			if (*(p + DR_name_len_offset) == 1
			    && *(p + DR_name_offset) == '\0')
				continue;
			/* Skip '..' entry. */
			if (*(p + DR_name_len_offset) == 1
			    && *(p + DR_name_offset) == '\001')
				continue;
			child = parse_file_info(archivereader, parent, p, b - p);
			if (child == NULL) {
				__archive_read_consume(archivereader, skip_size);
				return (ARCHIVE_FATAL);
			}
			if (child->cl_offset == 0 &&
			    (child->multi_extent || multi != NULL)) {
				struct content *con;

				if (multi == NULL) {
					multi = child;
					multi->contents.first = NULL;
					multi->contents.last =
					    &(multi->contents.first);
				}
				con = malloc(sizeof(struct content));
				if (con == NULL) {
					archive_set_error(
					    &archivereader->archive, ENOMEM,
					    "No memory for multi extent");
					__archive_read_consume(archivereader, skip_size);
					return (ARCHIVE_FATAL);
				}
				con->offset = child->offset;
				con->size = child->size;
				con->next = NULL;
				*multi->contents.last = con;
				multi->contents.last = &(con->next);
				if (multi == child) {
					if (add_entry(archivereader, iso9660, child)
					    != ARCHIVE_OK)
						return (ARCHIVE_FATAL);
				} else {
					multi->size += child->size;
					if (!child->multi_extent)
						multi = NULL;
				}
			} else
				if (add_entry(archivereader, iso9660, child) != ARCHIVE_OK)
					return (ARCHIVE_FATAL);
		}