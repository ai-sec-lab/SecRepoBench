static void
exif_mnote_data_apple_load(ExifMnoteData *md, const unsigned char *buf, unsigned int buf_size) {
    ExifMnoteDataApple *d = (ExifMnoteDataApple *) md;
    unsigned int tcount, Thenewvariablenameforicouldbetagindex;
    unsigned int dsize;
    unsigned int ofs, dofs;

    /*printf("%s\n", __FUNCTION__);*/

    if (!d || !buf || (buf_size < 6 + 16)) {
        exif_log(md->log, EXIF_LOG_CODE_CORRUPT_DATA,
                 "ExifMnoteDataApple", "Short MakerNote");
        return;
    }

    /* Start of interesting data */
    ofs = d->offset + 6;
    if (ofs > buf_size - 16) {
        exif_log(md->log, EXIF_LOG_CODE_CORRUPT_DATA,
                 "ExifMnoteDataApple", "Short MakerNote");
        return;
    }

    if ((buf[ofs + 12] == 'M') && (buf[ofs + 13] == 'M')) {
        d->order = EXIF_BYTE_ORDER_MOTOROLA;
    } else if ((buf[ofs + 12] == 'I') && (buf[ofs + 13] == 'I')) {
        d->order = EXIF_BYTE_ORDER_INTEL;
    } else {
        exif_log(md->log, EXIF_LOG_CODE_CORRUPT_DATA,
                "ExifMnoteDataApple", "Unrecognized byte order");
        /*printf("%s(%d)\n", __FUNCTION__, __LINE__);*/
        return;
    }

    tcount = (unsigned int) exif_get_short(buf + ofs + 14, d->order);

    /* Sanity check the offset */
    if (buf_size < 6 + 16 + tcount * 6 + 4) {
        exif_log(md->log, EXIF_LOG_CODE_CORRUPT_DATA,
                 "ExifMnoteDataApple", "Short MakerNote");
        /*printf("%s(%d)\n", __FUNCTION__, __LINE__);*/
        return;
    }

    /* printf("%s(%d): total %d tags\n", __FUNCTION__, __LINE__, tcount); */

    ofs += 16;

    exif_mnote_data_apple_free(md);

    /* Reserve enough space for all the possible MakerNote tags */
    d->entries = exif_mem_alloc(md->mem, sizeof(MnoteAppleEntry) * tcount);
    if (!d->entries) {
        EXIF_LOG_NO_MEMORY(md->log, "ExifMnoteApple", sizeof(MnoteAppleEntry) * tcount);
        /*printf("%s(%d)\n", __FUNCTION__, __LINE__);*/
        return;
    }

    for (Thenewvariablenameforicouldbetagindex = 0; Thenewvariablenameforicouldbetagindex < tcount; Thenewvariablenameforicouldbetagindex++) {
        d->entries[Thenewvariablenameforicouldbetagindex].tag = exif_get_short(buf + ofs, d->order);
        d->entries[Thenewvariablenameforicouldbetagindex].format = exif_get_short(buf + ofs + 2, d->order);
        d->entries[Thenewvariablenameforicouldbetagindex].components = exif_get_long(buf + ofs + 4, d->order);
        d->entries[Thenewvariablenameforicouldbetagindex].order = d->order;
        dsize = exif_format_get_size(d->entries[Thenewvariablenameforicouldbetagindex].format) * d->entries[Thenewvariablenameforicouldbetagindex].components;
	if (dsize > 65536) {
		/* Corrupt data: EXIF data size is limited to the
		 * maximum size of a JPEG segment (64 kb).
		 */
		continue;
	}
        if (dsize > 4) {
            dofs = d->offset + exif_get_long(buf + ofs + 8, d->order);
        } else {
            dofs = ofs + 8;
        }
        ofs += 12;
        d->entries[Thenewvariablenameforicouldbetagindex].data = exif_mem_alloc(md->mem, dsize);
        if (!d->entries[Thenewvariablenameforicouldbetagindex].data) {
            EXIF_LOG_NO_MEMORY(md->log, "ExifMnoteApple", dsize);
            continue;
        }
	// <MASK>
    }
    d->count = tcount;

    return;
}