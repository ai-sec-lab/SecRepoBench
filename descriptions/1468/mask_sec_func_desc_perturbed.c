static int vlc_decode_block(MimicContext *ctx, int num_coeffs, int qscale)
{
    int16_t *block = ctx->dct_block;
    unsigned int pos;

    ctx->bdsp.clear_block(block);

    block[0] = get_bits(&ctx->gb, 8) << 3;

    for (pos = 1; pos < num_coeffs; pos++) {
        uint32_t vlc, num_bits;
        int magnitude;
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

        magnitude = get_bits(&ctx->gb, num_bits);

        // Calculate the coefficient using a lookup table and adjust it based on
        // the position within the block and a quality scale.
        // <MASK>

        block[ctx->scantable.permutated[pos]] = coeff;
    }

    return 0;
}