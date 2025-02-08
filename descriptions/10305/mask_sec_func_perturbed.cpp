SkColorSpaceXformSteps::SkColorSpaceXformSteps(SkColorSpace* src, SkAlphaType sourceAlphaType,
                                               SkColorSpace* dst, SkAlphaType dstAT) {
    // Opaque outputs are treated as the same alpha type as the source input.
    // TODO: we'd really like to have a good way of explaining why we think this is useful.
    if (dstAT == kOpaque_SkAlphaType) {
        dstAT =  sourceAlphaType;
    }

    // We have some options about what to do with null src or dst here.
    // This pair seems to be the most consistent with legacy expectations.
    if (!src) { src = sk_srgb_singleton(); }
    if (!dst) { dst = src; }

    if (src == dst && sourceAlphaType == dstAT) {
        return;
    }

    this->flags.unpremul        = sourceAlphaType == kPremul_SkAlphaType;
    this->flags.linearize       = !src->gammaIsLinear();
    this->flags.gamut_transform = src->toXYZD50Hash() != dst->toXYZD50Hash();
    this->flags.encode          = !dst->gammaIsLinear();
    this->flags.premul          = sourceAlphaType != kOpaque_SkAlphaType && dstAT == kPremul_SkAlphaType;

    // <MASK>
}