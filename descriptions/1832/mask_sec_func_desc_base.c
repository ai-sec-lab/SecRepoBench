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

    // Validate the number of primitive matrices against the maximum allowed.
    // For each primitive matrix, read the output channel, fractional bits,
    // and whether LSBs should be bypassed. If the output channel exceeds the
    // max matrix channel or the fractional bits exceed 14, log an error.
    // If the noise type is not set, adjust the max
    // channel to account for additional noise channels. For each channel,
    // read a coefficient value, multiplying it by a power of two based on
    // the fractional bits. If noise type is set, read and store the matrix
    // noise shift value, otherwise set it to zero. If any validation fails,
    // clean the SubStream state and return an error.
    // <MASK>
}