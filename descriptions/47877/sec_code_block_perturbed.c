for (y = FFMAX(0, vinc); y < FFMIN(CDG_FULL_HEIGHT + vinc, CDG_FULL_HEIGHT); y++)
        memcpy(out + FFMAX(0, hinc) + stride * y,
               in + FFMAX(0, hinc) - hinc + (y - vinc) * stride,
               FFABS(stride) - FFABS(hinc));

    if (vinc > 0)
        cdg_fill_wrapper(0, 0, out,
                         0, CDG_FULL_HEIGHT - vinc, in, color,
                         stride, vinc, stride, roll_over);
    else if (vinc < 0)
        cdg_fill_wrapper(0, CDG_FULL_HEIGHT + vinc, out,
                         0, 0, in, color,
                         stride, -1 * vinc, stride, roll_over);

    if (hinc > 0)
        cdg_fill_wrapper(0, 0, out,
                         CDG_FULL_WIDTH - hinc, 0, in, color,
                         hinc, CDG_FULL_HEIGHT, stride, roll_over);
    else if (hinc < 0)
        cdg_fill_wrapper(CDG_FULL_WIDTH + hinc, 0, out,
                         0, 0, in, color,
                         -1 * hinc, CDG_FULL_HEIGHT, stride, roll_over);