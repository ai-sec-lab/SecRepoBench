SkColorSpaceXformSteps::SkColorSpaceXformSteps(SkColorSpace* src, SkAlphaType srcAT,
                                               SkColorSpace* dst, SkAlphaType dstAT) {
    // Opaque outputs are treated as the same alpha type as the source input.
    // TODO: we'd really like to have a good way of explaining why we think this is useful.
    if (dstAT == kOpaque_SkAlphaType) {
        dstAT =  srcAT;
    }

    // We have some options about what to do with null src or dst here.
    // This pair seems to be the most consistent with legacy expectations.
    if (!src) { src = sk_srgb_singleton(); }
    if (!dst) { dst = src; }

    if (src == dst && srcAT == dstAT) {
        return;
    }

    this->flags.unpremul        = srcAT == kPremul_SkAlphaType;
    this->flags.linearize       = !src->gammaIsLinear();
    this->flags.gamut_transform = src->toXYZD50Hash() != dst->toXYZD50Hash();
    this->flags.encode          = !dst->gammaIsLinear();
    this->flags.premul          = srcAT != kOpaque_SkAlphaType && dstAT == kPremul_SkAlphaType;

    if (this->flags.gamut_transform) {
        auto xform = SkMatrix44(*dst->fromXYZD50(), *src->toXYZD50());
        for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++) {
            this->src_to_dst_matrix[3*c+r] = xform.get(r,c);
        }
    }

    // Fill out all the transfer functions we'll use:
    SkColorSpaceTransferFn srcTF, dstTF;
    SkAssertResult(src->isNumericalTransferFn(&srcTF));
    SkAssertResult(dst->isNumericalTransferFn(&dstTF));
    this->srcTF         = srcTF;
    this->dstTFInv      = dstTF.invert();
    this->srcTF_is_sRGB = src->gammaCloseToSRGB();
    this->dstTF_is_sRGB = dst->gammaCloseToSRGB();

    // If we linearize then immediately reencode with the same transfer function, skip both.
    if ( this->flags.linearize       &&
        !this->flags.gamut_transform &&
         this->flags.encode          &&
        0 == memcmp(&srcTF, &dstTF, sizeof(SkColorSpaceTransferFn)))
    {
        this->flags.linearize  = false;
        this->flags.encode     = false;
    }

    // Skip unpremul...premul if there are no non-linear operations between.
    if ( this->flags.unpremul   &&
        !this->flags.linearize  &&
        !this->flags.encode     &&
         this->flags.premul)
    {
        this->flags.unpremul = false;
        this->flags.premul   = false;
    }
}