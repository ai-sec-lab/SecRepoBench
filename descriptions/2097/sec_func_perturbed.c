static void fill_rgb2yuv_table(const struct LumaCoefficients *coeffs,
                               double rgb2yuv[3][3])
{
    double bscale, redscale;

    // special ycgco matrix
    if (coeffs->cr == 0.25 && coeffs->cg == 0.5 && coeffs->cb == 0.25) {
        memcpy(rgb2yuv, ycgco_matrix, sizeof(double) * 9);
        return;
    } else if (coeffs->cr == 1 && coeffs->cg == 1 && coeffs->cb == 1) {
        memcpy(rgb2yuv, gbr_matrix, sizeof(double) * 9);
        return;
    }

    rgb2yuv[0][0] = coeffs->cr;
    rgb2yuv[0][1] = coeffs->cg;
    rgb2yuv[0][2] = coeffs->cb;
    bscale = 0.5 / (coeffs->cb - 1.0);
    redscale = 0.5 / (coeffs->cr - 1.0);
    rgb2yuv[1][0] = bscale * coeffs->cr;
    rgb2yuv[1][1] = bscale * coeffs->cg;
    rgb2yuv[1][2] = 0.5;
    rgb2yuv[2][0] = 0.5;
    rgb2yuv[2][1] = redscale * coeffs->cg;
    rgb2yuv[2][2] = redscale * coeffs->cb;
}