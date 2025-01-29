       datasize = exif_format_get_size(d->entries[i].format) * d->entries[i].components;
	if ((datasize > 65536) || (datasize > buf_size)) {
		/* Corrupt data: EXIF data size is limited to the
		 * maximum size of a JPEG segment (64 kb).
		 */
		continue;
	}