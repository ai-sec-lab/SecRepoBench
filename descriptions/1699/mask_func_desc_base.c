static int read_matrix_params(MLPDecodeContext *m, unsigned int substr, GetBitContext *gbp)
{
    SubStream *s = &m->substream[substr];
    unsigned int mat, ch;
    const int max_primitive_matrices = m->avctx->codec_id == AV_CODEC_ID_MLP
                                     ? MAX_MATRICES_MLP
                                     : MAX_MATRICES_TRUEHD;

    if (m->matrix_changed++ > 1) {
        av_log(m->avctx, AV_LOG_ERROR, "Matrices may change only once per access unit.\n");
        return AVERROR_INVALIDDATA;
    }

    s->num_primitive_matrices = get_bits(gbp, 4);

    // Check if the number of primitive matrices exceeds the maximum allowed.
    // If it does, log an error message and then return an error. For each matrix, read the matrix
    // output channel, the number of fractional bits, and whether LSB bypass is
    // enabled. Validate matrix output channel and fractional bits. For each
    // channel up to the maximum matrix channel, read and store the matrix
    // coefficient values. If noise is present, read the matrix noise shift value
    // for each matrix; otherwise, set it to zero.
    // <MASK>

    for (mat = 0; mat < s->num_primitive_matrices; mat++) {
        int frac_bits, max_chan;
        s->matrix_out_ch[mat] = get_bits(gbp, 4);
        frac_bits             = get_bits(gbp, 4);
        s->lsb_bypass   [mat] = get_bits1(gbp);

        if (s->matrix_out_ch[mat] > s->max_matrix_channel) {
            av_log(m->avctx, AV_LOG_ERROR,
                    "Invalid channel %d specified as output from matrix.\n",
                    s->matrix_out_ch[mat]);
            return AVERROR_INVALIDDATA;
        }
        if (frac_bits > 14) {
            av_log(m->avctx, AV_LOG_ERROR,
                    "Too many fractional bits specified.\n");
            return AVERROR_INVALIDDATA;
        }

        max_chan = s->max_matrix_channel;
        if (!s->noise_type)
            max_chan+=2;

        for (ch = 0; ch <= max_chan; ch++) {
            int coeff_val = 0;
            if (get_bits1(gbp))
                coeff_val = get_sbits(gbp, frac_bits + 2);

            s->matrix_coeff[mat][ch] = coeff_val * (1 << (14 - frac_bits));
        }

        if (s->noise_type)
            s->matrix_noise_shift[mat] = get_bits(gbp, 4);
        else
            s->matrix_noise_shift[mat] = 0;
    }

    return 0;
}