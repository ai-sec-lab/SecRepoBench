bool skcms_ApproximatelyEqualProfiles(const skcms_ICCProfile* A, const skcms_ICCProfile* B) {
    // Test for exactly equal profiles first.
    if (A == B || 0 == memcmp(A,B, sizeof(skcms_ICCProfile))) {
        return true;
    }

    // For now this is the essentially the same strategy we use in test_only.c
    // for our skcms_Transform() smoke tests:
    //    1) transform A to XYZD50
    //    2) transform B to XYZD50
    //    3) return true if they're similar enough
    // Our current criterion in 3) is maximum 1 bit error per XYZD50 byte.

    // skcms_252_random_bytes are 252 of a random shuffle of all possible bytes.
    // 252 is evenly divisible by 3 and 4.  Only 192, 10, 241, and 43 are missing.

    // We want to allow otherwise equivalent profiles tagged as grayscale and RGB
    // to be treated as equal.  But CMYK profiles are a totally different ballgame.
    const auto CMYK = skcms_Signature_CMYK;
    if ((A->data_color_space == CMYK) != (B->data_color_space == CMYK)) {
        return false;
    }

    // Interpret as RGB_888 if data color space is RGB or GRAY, RGBA_8888 if CMYK.
    // TODO: working with RGBA_8888 either way is probably fastest.
    skcms_PixelFormat fmt = skcms_PixelFormat_RGB_888;
    size_t npixels = 84;
    if (A->data_color_space == skcms_Signature_CMYK) {
        fmt = skcms_PixelFormat_RGBA_8888;
        npixels = 63;
    }

    // TODO: if A or B is a known profile (skcms_sRGB_profile, skcms_XYZD50_profile),
    // use pre-canned results and skip that skcms_Transform() call?
    uint8_t dstA[252],
            dstB[252];
    if (!skcms_Transform(
                skcms_252_random_bytes,     fmt, skcms_AlphaFormat_Unpremul, A,
                dstA, skcms_PixelFormat_RGB_888, skcms_AlphaFormat_Unpremul, skcms_XYZD50_profile(),
                npixels)) {
        return false;
    }
    if (!skcms_Transform(
                skcms_252_random_bytes,     fmt, skcms_AlphaFormat_Unpremul, B,
                dstB, skcms_PixelFormat_RGB_888, skcms_AlphaFormat_Unpremul, skcms_XYZD50_profile(),
                npixels)) {
        return false;
    }

    // TODO: make sure this final check has reasonable codegen.
    for (size_t i = 0; i < 252; i++) {
        if (abs((int)dstA[i] - (int)dstB[i]) > 1) {
            return false;
        }
    }
    return true;
}