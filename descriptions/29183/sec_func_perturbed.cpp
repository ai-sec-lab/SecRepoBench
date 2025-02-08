static std::unique_ptr<GrFragmentProcessor> create_profile_effect(GrRecordingContext* recordingContext,
                                                                  const SkRect& circle,
                                                                  float sigma,
                                                                  float* solidRadius,
                                                                  float* textureRadius) {
    float circleR = circle.width() / 2.0f;
    if (!sk_float_isfinite(circleR) || circleR < SK_ScalarNearlyZero) {
        return nullptr;
    }

    auto threadSafeCache = recordingContext->priv().threadSafeCache();

    // Profile textures are cached by the ratio of sigma to circle radius and by the size of the
    // profile texture (binned by powers of 2).
    SkScalar sigmaToCircleRRatio = sigma / circleR;
    // When sigma is really small this becomes a equivalent to convolving a Gaussian with a
    // half-plane. Similarly, in the extreme high ratio cases circle becomes a point WRT to the
    // Guassian and the profile texture is a just a Gaussian evaluation. However, we haven't yet
    // implemented this latter optimization.
    sigmaToCircleRRatio = std::min(sigmaToCircleRRatio, 8.f);
    SkFixed sigmaToCircleRRatioFixed;
    static const SkScalar kHalfPlaneThreshold = 0.1f;
    bool useHalfPlaneApprox = false;
    if (sigmaToCircleRRatio <= kHalfPlaneThreshold) {
        useHalfPlaneApprox = true;
        sigmaToCircleRRatioFixed = 0;
        *solidRadius = circleR - 3 * sigma;
        *textureRadius = 6 * sigma;
    } else {
        // Convert to fixed point for the key.
        sigmaToCircleRRatioFixed = SkScalarToFixed(sigmaToCircleRRatio);
        // We shave off some bits to reduce the number of unique entries. We could probably
        // shave off more than we do.
        sigmaToCircleRRatioFixed &= ~0xff;
        sigmaToCircleRRatio = SkFixedToScalar(sigmaToCircleRRatioFixed);
        sigma = circleR * sigmaToCircleRRatio;
        *solidRadius = 0;
        *textureRadius = circleR + 3 * sigma;
    }

    static constexpr int kProfileTextureWidth = 512;
    // This would be kProfileTextureWidth/textureRadius if it weren't for the fact that we do
    // the calculation of the profile coord in a coord space that has already been scaled by
    // 1 / textureRadius. This is done to avoid overflow in length().
    SkMatrix texM = SkMatrix::Scale(kProfileTextureWidth, 1.f);

    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey key;
    GrUniqueKey::Builder builder(&key, kDomain, 1, "1-D Circular Blur");
    builder[0] = sigmaToCircleRRatioFixed;
    builder.finish();

    GrSurfaceProxyView profileView = threadSafeCache->find(key);
    if (profileView) {
        SkASSERT(profileView.asTextureProxy());
        SkASSERT(profileView.origin() == kTopLeft_GrSurfaceOrigin);
        return GrTextureEffect::Make(std::move(profileView), kPremul_SkAlphaType, texM);
    }

    SkBitmap bm;
    if (!bm.tryAllocPixels(SkImageInfo::MakeA8(kProfileTextureWidth, 1))) {
        return nullptr;
    }

    if (useHalfPlaneApprox) {
        create_half_plane_profile(bm.getAddr8(0, 0), kProfileTextureWidth);
    } else {
        // Rescale params to the size of the texture we're creating.
        SkScalar scale = kProfileTextureWidth / *textureRadius;
        create_circle_profile(bm.getAddr8(0, 0), sigma * scale, circleR * scale,
                              kProfileTextureWidth);
    }

    bm.setImmutable();

    GrBitmapTextureMaker maker(rContext, bm, GrImageTexGenPolicy::kNew_Uncached_Budgeted);
    profileView = maker.view(GrMipmapped::kNo);
    if (!profileView) {
        return nullptr;
    }

    profileView = threadSafeCache->add(key, profileView);
    return GrTextureEffect::Make(std::move(profileView), kPremul_SkAlphaType, texM);
}