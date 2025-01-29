builder.finish();

    GrProxyProvider* provider = ctx->priv().proxyProvider();
    GrSurfaceProxyView view;
    if (auto proxy = provider->findProxyByUniqueKey(key)) {
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