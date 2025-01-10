
        uint32_t vlc, num_bits;
        int value;
        int coeff;

        vlc = get_vlc2(&ctx->gb, ctx->vlc.table, ctx->vlc.bits, 3);
        if (!vlc) /* end-of-block code */
            return 0;
        if (vlc == -1)
            return AVERROR_INVALIDDATA;

        /* pos_add and num_bits are coded in the vlc code */
        pos     += vlc & 15; // pos_add
        num_bits = vlc >> 4; // num_bits

        if (pos >= 64)
            return AVERROR_INVALIDDATA;

        value = get_bits(&ctx->gb, num_bits);

        /* FFmpeg's IDCT behaves somewhat different from the original code, so
         * a factor of 4 was added to the input */

        coeff = vlcdec_lookup[num_bits][value];
        if (pos < 3)
            coeff *= 16;
        else /* TODO Use >> 10 instead of / 1001 */
            coeff = (coeff * qscale) / 1001;

        block[ctx->scantable.permutated[pos]] = coeff;
    