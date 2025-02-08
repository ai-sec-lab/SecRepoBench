// We want to allow otherwise equivalent profiles tagged as grayscale and RGB
    // to be treated as equal.  But CMYK profiles are a totally different ballgame.
    const auto CMYK = skcms_Signature_CMYK;
    if ((profileA->data_color_space == CMYK) != (B->data_color_space == CMYK)) {
        return false;
    }