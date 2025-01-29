
    SkVector scale = SkVector::Make(fScale, fScale);
    ctx.ctm().mapVectors(&scale, 1);

#if SK_SUPPORT_GPU
    if (source->isTextureBacked()) {
        GrContext* context = source->getContext();

        sk_sp<GrTextureProxy> colorProxy(color->asTextureProxyRef(context));
        sk_sp<GrTextureProxy> displProxy(displ->asTextureProxyRef(context));
        if (!colorProxy || !displProxy) {
            return nullptr;
        }

        SkMatrix offsetMatrix = SkMatrix::MakeTrans(SkIntToScalar(colorOffset.fX - displOffset.fX),
                                                    SkIntToScalar(colorOffset.fY - displOffset.fY));
        SkColorSpace* colorSpace = ctx.outputProperties().colorSpace();

        GrPixelConfig colorConfig = colorProxy->config();
        std::unique_ptr<GrFragmentProcessor> fp =
                GrDisplacementMapEffect::Make(fXChannelSelector,
                                              fYChannelSelector,
                                              scale,
                                              std::move(displProxy),
                                              offsetMatrix,
                                              std::move(colorProxy),
                                              SkISize::Make(color->width(), color->height()));
        fp = GrColorSpaceXformEffect::Make(std::move(fp), color->getColorSpace(), colorConfig,
                                           colorSpace);

        GrPaint paint;
        paint.addColorFragmentProcessor(std::move(fp));
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
        SkMatrix matrix;
        matrix.setTranslate(-SkIntToScalar(colorBounds.x()), -SkIntToScalar(colorBounds.y()));

        sk_sp<GrRenderTargetContext> renderTargetContext(
            context->contextPriv().makeDeferredRenderTargetContext(SkBackingFit::kApprox,
                                                     bounds.width(), bounds.height(),
                                                     GrRenderableConfigForColorSpace(colorSpace),
                                                     sk_ref_sp(colorSpace)));
        if (!renderTargetContext) {
            return nullptr;
        }
        paint.setGammaCorrect(renderTargetContext->colorSpaceInfo().isGammaCorrect());

        renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, matrix,
                                      SkRect::Make(colorBounds));

        offset->fX = bounds.left();
        offset->fY = bounds.top();
        return SkSpecialImage::MakeDeferredFromGpu(
                context,
                SkIRect::MakeWH(bounds.width(), bounds.height()),
                kNeedNewImageUniqueID_SpecialImage,
                renderTargetContext->asTextureProxyRef(),
                renderTargetContext->colorSpaceInfo().refColorSpace());
    }
#endif

    SkBitmap colorBM, displBM;

    if (!color->getROPixels(&colorBM) || !displ->getROPixels(&displBM)) {
        return nullptr;
    }

    if ((colorBM.colorType() != kN32_SkColorType) ||
        (displBM.colorType() != kN32_SkColorType)) {
        return nullptr;
    }

    if (!colorBM.getPixels() || !displBM.getPixels()) {
        return nullptr;
    }

    SkImageInfo info = SkImageInfo::MakeN32(bounds.width(), bounds.height(),
                                            colorBM.alphaType());

    SkBitmap dst;
    if (!dst.tryAllocPixels(info)) {
        return nullptr;
    }

    computeDisplacement(Extractor(fXChannelSelector, fYChannelSelector), scale, &dst,
                        displBM, colorOffset - displOffset, colorBM, colorBounds);

    offset->fX = bounds.left();
    offset->fY = bounds.top();
    return SkSpecialImage::MakeFromRaster(SkIRect::MakeWH(bounds.width(), bounds.height()),
                                          dst);