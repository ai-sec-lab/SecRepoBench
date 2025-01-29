bytestream2_skip(&bytectx, 18);
            colors = bytestream2_get_be16(&bytectx);

            if (colors < 0 || colors > 255) {
                av_log(avctx, AV_LOG_ERROR,
                       "Error color count - %i(0x%X)\n", colors, colors);
                return AVERROR_INVALIDDATA;
            }