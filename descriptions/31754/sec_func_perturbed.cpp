std::unique_ptr<GrFragmentProcessor> SkPictureShader::asFragmentProcessor(
        const GrFPArgs& args) const {

    auto ctx = args.fContext;
    auto lm = this->totalLocalMatrix(args.fPreLocalMatrix);
    SkColorType dstColorType = GrColorTypeToSkColorType(args.fDstColorInfo->colorType());
    if (dstColorType == kUnknown_SkColorType) {
        dstColorType = kRGBA_8888_SkColorType;
    }

    auto dstCS = ref_or_srgb(args.fDstColorInfo->colorSpace());
    auto info = CachedImageInfo::Make(fTile, args.fMatrixProvider.localToDevice(), &lm,
                                      dstColorType, dstCS.get(),
                                      ctx->priv().caps()->maxTextureSize());
    SkMatrix inverseMatrix;
    if (!info.success || !(*lm).invert(&inverseMatrix)) {
        return nullptr;
    }

    // Gotta be sure the GPU can support our requested colortype (might be FP16)
    if (!ctx->colorTypeSupportedAsSurface(info.imageInfo.colorType())) {
        info.imageInfo = info.imageInfo.makeColorType(kRGBA_8888_SkColorType);
    }

    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey key;
    GrUniqueKey::Builder builder(&key, kDomain, 10, "Picture Shader Image");
    builder[0] = dstCS->toXYZD50Hash();
    builder[1] = dstCS->transferFnHash();
    builder[2] = static_cast<uint32_t>(dstColorType);
    builder[3] = fPicture->uniqueID();
    memcpy(&builder[4], &fTile, sizeof(fTile));                     // 4,5,6,7
    memcpy(&builder[8], &info.tileScale, sizeof(info.tileScale));   // 8,9
    builder.finish();

    GrProxyProvider* provider = ctx->priv().proxyProvider();
    GrSurfaceProxyView view;
    if (auto proxy = provider->findOrCreateProxyByUniqueKey(key)) {
        view = GrSurfaceProxyView(proxy, kTopLeft_GrSurfaceOrigin, GrSwizzle());
    } else {
        const int msaaSampleCount = 0;
        const SkSurfaceProps* props = nullptr;
        const bool createWithMips = false;
        auto image = info.makeImage(SkSurface::MakeRenderTarget(ctx,
                                                                SkBudgeted::kYes,
                                                                info.imageInfo,
                                                                msaaSampleCount,
                                                                kTopLeft_GrSurfaceOrigin,
                                                                props,
                                                                createWithMips),
                                    fPicture.get());
        if (!image) {
            return nullptr;
        }
        auto [v, ct] = as_IB(image)->asView(ctx, GrMipmapped::kNo);
        view = std::move(v);
        provider->assignUniqueKeyToProxy(key, view.asTextureProxy());
    }

    const GrSamplerState sampler(static_cast<GrSamplerState::WrapMode>(fTmx),
                                 static_cast<GrSamplerState::WrapMode>(fTmy),
                                 sampling_to_filter(args.fSampling));
    return GrTextureEffect::Make(std::move(view), kPremul_SkAlphaType, inverseMatrix, sampler,
                                 *ctx->priv().caps());
}