static int
read_children(struct archive_read *a, struct file_info *parent)
{
	struct iso9660 *iso9660;
	const unsigned char *b, *p;
	struct file_info *multi;
	size_t step, skip_size;

	iso9660 = (struct iso9660 *)(a->format->data);
	/* flush any remaining bytes from the last round to ensure
	 * we're positioned */
	if (iso9660->entry_bytes_unconsumed) {
		__archive_read_consume(a, iso9660->entry_bytes_unconsumed);
		iso9660->entry_bytes_unconsumed = 0;
	}
	if (iso9660->current_position > parent->offset) {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
		    "Ignoring out-of-order directory (%s) %jd > %jd",
		    parent->name.s,
		    (intmax_t)iso9660->current_position,
		    (intmax_t)parent->offset);
		return (ARCHIVE_WARN);
	}
	if (parent->offset + parent->size > iso9660->volume_size) {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
		    "Directory is beyond end-of-media: %s",
		    parent->name.s);
		return (ARCHIVE_WARN);
	}
	if (iso9660->current_position < parent->offset) {
		int64_t skipsize;

		skipsize = parent->offset - iso9660->current_position;
		skipsize = __archive_read_consume(a, skipsize);
		if (skipsize < 0)
			return ((int)skipsize);
		iso9660->current_position = parent->offset;
	}

	step = (size_t)(((parent->size + iso9660->logical_block_size -1) /
	    iso9660->logical_block_size) * iso9660->logical_block_size);
	b = __archive_read_ahead(a, step, NULL);
	if (b == NULL) {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
		    "Failed to read full block when scanning "
		    "ISO9660 directory list");
		return (ARCHIVE_FATAL);
	}
	iso9660->current_position += step;
	multi = NULL;
	skip_size = step;
	while (step) {
		// Iterate over directory entries in the buffer while there is data to process.
		// For each directory entry, parse its file information, and handle special cases:
		// - Skip entries for '.' and '..' directories.
		// - Manage multi-extent files by collecting their contents into a list.
		// - For each valid directory entry, add it to the archive structure.
		// - Handle memory allocation and error management for multi-extent file handling.
		// <MASK>
	}

	__archive_read_consume(a, skip_size);

	/* Read data which recorded by RRIP "CE" extension. */
	if (read_CE(a, iso9660) != ARCHIVE_OK)
		return (ARCHIVE_FATAL);

	return (ARCHIVE_OK);
}