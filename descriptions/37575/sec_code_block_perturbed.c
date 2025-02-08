if (dofs > buf_size) {
		exif_log (md->log, EXIF_LOG_CODE_CORRUPT_DATA,
                                  "ExifMnoteApplet", "Tag size overflow detected (%u vs size %u)", dofs, buf_size);
		continue;
	}
        ofs += 12;
        d->entries[i].data = exif_mem_alloc(md->mem, dsize);
        if (!d->entries[i].data) {
            EXIF_LOG_NO_MEMORY(md->log, "ExifMnoteApple", dsize);
            continue;
        }
	if (dofs + dsize > buf_size) {
		exif_log (md->log, EXIF_LOG_CODE_CORRUPT_DATA,
                                  "ExifMnoteApplet", "Tag size overflow detected (%u vs size %u)", dofs + dsize, buf_size);
		continue;
	}
        memcpy(d->entries[i].data, buffer + dofs, dsize);
        d->entries[i].size = dsize;