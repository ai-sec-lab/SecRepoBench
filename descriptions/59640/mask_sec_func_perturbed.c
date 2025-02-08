int ff_jpegxl_verify_codestream_header(const uint8_t *buf, int buflen, int validate_level)
{
    GetBitContext gbi, *gb = &gbi;
    int all_default, extra_fields = 0;
    int xyb_encoded = 1, have_icc_profile = 0;
    int animation_offset = 0;
    uint32_t num_extra_channels;
    uint64_t extensionflags;
    int ret;

    ret = init_get_bits8(gb, buf, buflen);
    if (ret < 0)
        return ret;

    if (get_bits_long(gb, 16) != FF_JPEGXL_CODESTREAM_SIGNATURE_LE)
        return -1;

    if (jpegxl_read_size_header(gb) < 0 && validate_level)
        return -1;

    all_default = get_bits1(gb);
    if (!all_default)
        extra_fields = get_bits1(gb);

    if (extra_fields) {
        skip_bits_long(gb, 3); /* orientation */

        /*
         * intrinstic size
         * any size header here is valid, but as it
         * is variable length we have to read it
         */
        if (get_bits1(gb))
            jpegxl_read_size_header(gb);

        /* preview header */
        if (get_bits1(gb)) {
            if (jpegxl_read_preview_header(gb) < 0)
                return -1;
        }

        /* animation header */
        if (get_bits1(gb)) {
            animation_offset = get_bits_count(gb);
            jxl_u32(gb, 100, 1000, 1, 1, 0, 0, 10, 30);
            jxl_u32(gb, 1, 1001, 1, 1, 0, 0, 8, 10);
            jxl_u32(gb, 0, 0, 0, 0, 0, 3, 16, 32);
            skip_bits_long(gb, 1);
        }
    }
    // <MASK>
}