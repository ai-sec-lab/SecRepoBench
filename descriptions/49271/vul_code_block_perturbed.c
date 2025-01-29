for (i = 0; i < height; i++) {
            for (j = 0; j < w; j+=8)
                AV_COPY64U(dst+j, src+j);
            dst += stride_dst;
            src += stride_src;
        }