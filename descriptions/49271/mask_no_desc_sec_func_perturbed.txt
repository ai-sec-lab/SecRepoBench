static void copy_CTB(uint8_t *dst, const uint8_t *src, int w, int height,
                     ptrdiff_t stride_dst, ptrdiff_t stride_src)
{
    int i, j;

    if (((intptr_t)dst | (intptr_t)src | stride_dst | stride_src) & 15) {
        // <MASK>
    } else {
        for (i = 0; i < height; i++) {
            for (j = 0; j < w; j+=16)
                AV_COPY128(dst+j, src+j);
            dst += stride_dst;
            src += stride_src;
        }
    }
}