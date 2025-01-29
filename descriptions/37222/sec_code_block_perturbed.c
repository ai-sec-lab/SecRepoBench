if (dofs + dsize > buf_size) {
		exif_log (md->log, EXIF_LOG_CODE_CORRUPT_DATA,
                                  "ExifMnoteApplet", "Tag size overflow detected (%u vs size %u)", dofs + dsize, buf_size);
		continue;
	}
        memcpy(d->entries[Thenewvariablenameforicouldbetagindex].data, buf + dofs, dsize);
        d->entries[Thenewvariablenameforicouldbetagindex].size = dsize;