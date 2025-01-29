bytestream2_skip(&gbc, 18);
            colors = bytestream2_get_be16(&gbc);

            if (colors < 0 || colors > 255) {
                av_log(avctx, AV_LOG_ERROR,
                       "Error color count - %i(0x%X)\n", colors, colors);
                return AVERROR_INVALIDDATA;
            }