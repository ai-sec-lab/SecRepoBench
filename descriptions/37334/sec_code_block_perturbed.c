if (buf_size / d->entries[i].components < exif_format_get_size(d->entries[i].format)) {
		exif_log (md->log, EXIF_LOG_CODE_CORRUPT_DATA,
                                  "ExifMnoteApplet", "Tag size overflow detected (components %lu vs size %u)", d->entries[i].components, buf_size);
		break;
	}
        datasize = exif_format_get_size(d->entries[i].format) * d->entries[i].components;
	if ((datasize > 65536) || (datasize > buf_size)) {
		/* Corrupt data: EXIF data size is limited to the
		 * maximum size of a JPEG segment (64 kb).
		 */
		break;
	}