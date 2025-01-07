
        for (i = 0; i < height; i++) {
            for (j = 0; j < width - 7; j+=8)
                AV_COPY64U(dst+j, src+j);
            dst += stride_dst;
            src += stride_src;
        }
        if (width&7) {
            dst += ((width>>3)<<3) - stride_dst * height;
            src += ((width>>3)<<3) - stride_src * height;
            width &= 7;
            for (i = 0; i < height; i++) {
                for (j = 0; j < width; j++)
                    dst[j] = src[j];
                dst += stride_dst;
                src += stride_src;
            }
        }
    