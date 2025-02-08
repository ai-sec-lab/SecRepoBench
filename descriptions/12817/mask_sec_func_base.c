static int parse_tables(struct archive_read* a, struct rar5* rar,
        const uint8_t* p)
{
    int ret, value, i, w, idx = 0;
    uint8_t bit_length[HUFF_BC],
        table[HUFF_TABLE_SIZE],
        nibble_mask = 0xF0,
        nibble_shift = 4;

    enum { ESCAPE = 15 };

    /* The data for table generation is compressed using a simple RLE-like
     * algorithm when storing zeroes, so we need to unpack it first. */
    // <MASK>

    ret = create_decode_tables(&table[idx], &rar->cstate.ld, HUFF_NC);
    if(ret != ARCHIVE_OK) {
        archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
                "Failed to create literal table");
        return ARCHIVE_FATAL;
    }

    idx += HUFF_NC;

    ret = create_decode_tables(&table[idx], &rar->cstate.dd, HUFF_DC);
    if(ret != ARCHIVE_OK) {
        archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
                "Failed to create distance table");
        return ARCHIVE_FATAL;
    }

    idx += HUFF_DC;

    ret = create_decode_tables(&table[idx], &rar->cstate.ldd, HUFF_LDC);
    if(ret != ARCHIVE_OK) {
        archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
                "Failed to create lower bits of distances table");
        return ARCHIVE_FATAL;
    }

    idx += HUFF_LDC;

    ret = create_decode_tables(&table[idx], &rar->cstate.rd, HUFF_RC);
    if(ret != ARCHIVE_OK) {
        archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
                "Failed to create repeating distances table");
        return ARCHIVE_FATAL;
    }

    return ARCHIVE_OK;
}