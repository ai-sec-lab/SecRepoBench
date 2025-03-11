static int read_matrix_params(MLPDecodeContext *m, unsigned int substr, GetBitContext *bit_context)
{
    SubStream *s = &m->substream[substr];
    unsigned int mat, ch;
    // Determine the maximum number of primitive matrices allowed based on the codec type.
    // Check if the matrix configuration has been changed more than once per access unit.
    // Retrieve the number of primitive matrices from the bitstream.
    // Validate that the number of primitive matrices does not exceed the allowed maximum.
    // If validation fails, log an error message and return an error code.
    // <MASK>

    for (mat = 0; mat < s->num_primitive_matrices; mat++) {
        int frac_bits, max_chan;
        s->matrix_out_ch[mat] = get_bits(bit_context, 4);
        frac_bits             = get_bits(bit_context, 4);
        s->lsb_bypass   [mat] = get_bits1(bit_context);

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
            if (get_bits1(bit_context))
                coeff_val = get_sbits(bit_context, frac_bits + 2);

            s->matrix_coeff[mat][ch] = coeff_val * (1 << (14 - frac_bits));
        }

        if (s->noise_type)
            s->matrix_noise_shift[mat] = get_bits(bit_context, 4);
        else
            s->matrix_noise_shift[mat] = 0;
    }

    return 0;
}