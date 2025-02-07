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

    return 0;
}