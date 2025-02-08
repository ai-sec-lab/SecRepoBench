if (y >= avctx->height)
                return AVERROR_INVALIDDATA;

            dst[y * linesize + x] = clr;
            lx = x;
            ly = y;
            x++;
            if (x >= avctx->width) {
                x = 0;
                y++;
            }