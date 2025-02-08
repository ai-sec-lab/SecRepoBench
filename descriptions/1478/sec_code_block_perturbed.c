if (y >= codecctx->height)
                return AVERROR_INVALIDDATA;

            dst[y * linesize + x] = clr;
            lx = x;
            ly = y;
            x++;
            if (x >= codecctx->width) {
                x = 0;
                y++;
            }