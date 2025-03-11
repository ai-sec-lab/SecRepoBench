cram_block_slice_hdr *cram_decode_slice_header(cram_fd *fd, cram_block *b) {
    cram_block_slice_hdr *slice_header;
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

    if (!(slice_header  = calloc(1, sizeof(*slice_header))))
        return NULL;

    slice_header->content_type = b->content_type;

    if (b->content_type == MAPPED_SLICE) {
        cp += safe_itf8_get((char *)cp,  (char *)cp_end, &slice_header->ref_seq_id);
#ifdef LARGE_POS
        cp += safe_ltf8_get((char *)cp,  (char *)cp_end, &slice_header->ref_seq_start);
        cp += safe_ltf8_get((char *)cp,  (char *)cp_end, &slice_header->ref_seq_span);
#else
        int32_t i32;
        cp += safe_itf8_get((char *)cp,  (char *)cp_end, &i32);
        slice_header->ref_seq_start = i32;
        cp += safe_itf8_get((char *)cp,  (char *)cp_end, &i32);
        slice_header->ref_seq_span = i32;
#endif
        if (slice_header->ref_seq_start < 0 || slice_header->ref_seq_span < 0) {
            free(slice_header);
            hts_log_error("Negative values not permitted for header "
                          "sequence start or span fields");
            return NULL;
        }
    }
    cp += safe_itf8_get((char *)cp,  (char *)cp_end, &slice_header->num_records);
    slice_header->record_counter = 0;
    if (CRAM_MAJOR_VERS(fd->version) == 2) {
        int32_t i32 = 0;
        cp += safe_itf8_get((char *)cp, (char *)cp_end, &i32);
        slice_header->record_counter = i32;
    } else if (CRAM_MAJOR_VERS(fd->version) >= 3) {
        cp += safe_ltf8_get((char *)cp, (char *)cp_end, &slice_header->record_counter);
    }

    cp += safe_itf8_get((char *)cp, (char *)cp_end, &slice_header->num_blocks);

    cp += safe_itf8_get((char *)cp, (char *)cp_end, &slice_header->num_content_ids);
    if (slice_header->num_content_ids < 1 ||
        slice_header->num_content_ids >= SIZE_MAX / sizeof(int32_t)) {
        /* Slice must have at least one data block,
           and malloc'd size shouldn't wrap. */
        free(slice_header);
        return NULL;
    }
    slice_header->block_content_ids = malloc(slice_header->num_content_ids * sizeof(int32_t));
    if (!slice_header->block_content_ids) {
        free(slice_header);
        return NULL;
    }

    for (i = 0; i < slice_header->num_content_ids; i++) {
        int l = safe_itf8_get((char *)cp, (char *)cp_end,
                              &slice_header->block_content_ids[i]);
        if (l <= 0) {
            free(slice_header->block_content_ids);
            free(slice_header);
            return NULL;
        }
        cp += l;
    }

    if (b->content_type == MAPPED_SLICE) {
        cp += safe_itf8_get((char *)cp, (char *) cp_end, &slice_header->ref_base_id);
    }

    if (CRAM_MAJOR_VERS(fd->version) != 1) {
        if (cp_end - cp < 16) {
            free(slice_header->block_content_ids);
            free(slice_header);
            return NULL;
        }
        memcpy(slice_header->md5, cp, 16);
    } else {
        memset(slice_header->md5, 0, 16);
    }

    return slice_header;
}