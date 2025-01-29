
    if (!all_default) {
        jpegxl_skip_bit_depth(gb);

        /* modular_16bit_buffers must equal 1 */
        if (!get_bits1(gb) && validate_level)
            return -1;

        num_extra_channels = jxl_u32(gb, 0, 1, 2, 1, 0, 0, 4, 12);
        if (num_extra_channels > 4 && validate_level)
            return -1;
        for (uint32_t i = 0; i < num_extra_channels; i++) {
            if (jpegxl_read_extra_channel_info(gb, validate_level) < 0)
                return -1;
        }

        xyb_encoded = get_bits1(gb);

        /* color encoding bundle */
        if (!get_bits1(gb)) {
            uint32_t color_space;
            have_icc_profile = get_bits1(gb);
            color_space = jxl_enum(gb);
            if (color_space > 63)
                return -1;

            if (!have_icc_profile) {
                if (color_space != FF_JPEGXL_CS_XYB) {
                    uint32_t white_point = jxl_enum(gb);
                    if (white_point > 63)
                        return -1;
                    if (white_point == FF_JPEGXL_WP_CUSTOM) {
                        /* ux and uy values */
                        jxl_u32(gb, 0, 524288, 1048576, 2097152, 19, 19, 20, 21);
                        jxl_u32(gb, 0, 524288, 1048576, 2097152, 19, 19, 20, 21);
                    }
                    if (color_space != FF_JPEGXL_CS_GRAY) {
                        /* primaries */
                        uint32_t primaries = jxl_enum(gb);
                        if (primaries > 63)
                            return -1;
                        if (primaries == FF_JPEGXL_PR_CUSTOM) {
                            /* ux/uy values for r,g,b */
                            for (int i = 0; i < 6; i++)
                                jxl_u32(gb, 0, 524288, 1048576, 2097152, 19, 19, 20, 21);
                        }
                    }
                }

                /* transfer characteristics */
                if (get_bits1(gb)) {
                    /* gamma */
                    skip_bits_long(gb, 24);
                } else {
                    /* transfer function */
                    if (jxl_enum(gb) > 63)
                        return -1;
                }

                /* rendering intent */
                if (jxl_enum(gb) > 63)
                    return -1;
            }
        }

        /* tone mapping bundle */
        if (extra_fields && !get_bits1(gb))
            skip_bits_long(gb, 16 + 16 + 1 + 16);

        extensions = jpegxl_u64(gb);
        if (extensions) {
            for (int i = 0; i < 64; i++) {
                if (extensions & (UINT64_C(1) << i))
                    jpegxl_u64(gb);
            }
        }
    }

    /* default transform */
    if (!get_bits1(gb)) {
        /* opsin inverse matrix */
        if (xyb_encoded && !get_bits1(gb))
            skip_bits_long(gb, 16 * 16);
        /* cw_mask and default weights */
        if (get_bits1(gb))
            skip_bits_long(gb, 16 * 15);
        if (get_bits1(gb))
            skip_bits_long(gb, 16 * 55);
        if (get_bits1(gb))
            skip_bits_long(gb, 16 * 210);
    }

    if (!have_icc_profile) {
        int bits_remaining = 7 - (get_bits_count(gb) - 1) % 8;
        if (bits_remaining && get_bits(gb, bits_remaining))
            return -1;
    }

    if (get_bits_left(gb) < 0)
        return -1;

    return animation_offset;