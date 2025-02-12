/* FFmpeg's IDCT behaves somewhat different from the original code, so
         * a factor of 4 was added to the input */

        coeff = vlcdec_lookup[num_bits][value];
        if (pos < 3)
            coeff *= 16;
        else /* TODO Use >> 10 instead of / 1001 */
            coeff = (coeff * qscale) / 1001;