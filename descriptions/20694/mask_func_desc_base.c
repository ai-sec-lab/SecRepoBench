cram_block_slice_hdr *cram_decode_slice_header(cram_fd *fd, cram_block *b) {
    cram_block_slice_hdr *hdr;
    unsigned char *cp;
    unsigned char *cp_end;
    int i;

    if (b->method != RAW) {
        /* Spec. says slice header should be RAW, but we can future-proof
           by trying to decode it if it isn't. */
        if (cram_uncompress_block(b) < 0)
            return NULL;
    }
    cp =  (unsigned char *)BLOCK_DATA(b);
    cp_end = cp + b->uncomp_size;

    if (b->content_type != MAPPED_SLICE &&
        b->content_type != UNMAPPED_SLICE)
        return NULL;

    if (!(hdr  = calloc(1, sizeof(*hdr))))
        return NULL;

    hdr->content_type = b->content_type;

    if (b->content_type == MAPPED_SLICE) {
        // Extract reference sequence ID, start, and span from the slice header.
        // This includes special handling for the LARGE_POS macro, which uses
        // larger data types for positions and spans. If LARGE_POS is defined,
        // use safe_ltf8_get to extract 64-bit values for ref_seq_start and
        // ref_seq_span; otherwise, use safe_itf8_get for standard 32-bit values.
        // <MASK>
    }
    cp += safe_itf8_get((char *)cp,  (char *)cp_end, &hdr->num_records);
    hdr->record_counter = 0;
    if (CRAM_MAJOR_VERS(fd->version) == 2) {
        int32_t i32 = 0;
        cp += safe_itf8_get((char *)cp, (char *)cp_end, &i32);
        hdr->record_counter = i32;
    } else if (CRAM_MAJOR_VERS(fd->version) >= 3) {
        cp += safe_ltf8_get((char *)cp, (char *)cp_end, &hdr->record_counter);
    }

    cp += safe_itf8_get((char *)cp, (char *)cp_end, &hdr->num_blocks);

    cp += safe_itf8_get((char *)cp, (char *)cp_end, &hdr->num_content_ids);
    if (hdr->num_content_ids < 1 ||
        hdr->num_content_ids >= SIZE_MAX / sizeof(int32_t)) {
        /* Slice must have at least one data block,
           and malloc'd size shouldn't wrap. */
        free(hdr);
        return NULL;
    }
    hdr->block_content_ids = malloc(hdr->num_content_ids * sizeof(int32_t));
    if (!hdr->block_content_ids) {
        free(hdr);
        return NULL;
    }

    for (i = 0; i < hdr->num_content_ids; i++) {
        int l = safe_itf8_get((char *)cp, (char *)cp_end,
                              &hdr->block_content_ids[i]);
        if (l <= 0) {
            free(hdr->block_content_ids);
            free(hdr);
            return NULL;
        }
        cp += l;
    }

    if (b->content_type == MAPPED_SLICE) {
        cp += safe_itf8_get((char *)cp, (char *) cp_end, &hdr->ref_base_id);
    }

    if (CRAM_MAJOR_VERS(fd->version) != 1) {
        if (cp_end - cp < 16) {
            free(hdr->block_content_ids);
            free(hdr);
            return NULL;
        }
        memcpy(hdr->md5, cp, 16);
    } else {
        memset(hdr->md5, 0, 16);
    }

    return hdr;
}