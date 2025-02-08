SkDrawTiler(SkBitmapDevice* dev, const SkRect* bounds) : fDevice(dev) {
        fDone = false;

        // we need fDst to be set, and if we're actually drawing, to dirty the genID
        if (!dev->accessPixels(&fRootPixmap)) {
            // NoDrawDevice uses us (why?) so we have to catch this case w/ no pixels
            fRootPixmap.reset(dev->imageInfo(), nullptr, 0);
        }

        // do a quick check, so we don't even have to process "bounds" if there is no need
        const SkIRect clipRect = dev->fRCStack.rc().getBounds();
        fNeedsTiling = clipRect.right() > kMaxDim || clipRect.bottom() > kMaxDim;
        if (fNeedsTiling) {
            if (bounds) // <MASK>
        }

        if (fNeedsTiling) {
            // fDraw.fDst is reset each time in setupTileDraw()
            fDraw.fMatrix = &fTileMatrix;
            fDraw.fRC = &fTileRC;
            // we'll step/increase it before using it
            fOrigin.set(fSrcBounds.fLeft - kMaxDim, fSrcBounds.fTop);
        } else {
            // don't reference fSrcBounds, as it may not have been set
            fDraw.fDst = fRootPixmap;
            fDraw.fMatrix = &dev->ctm();
            fDraw.fRC = &dev->fRCStack.rc();
            fOrigin.set(0, 0);
        }
    }