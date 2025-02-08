SkCodec::Result SkGifCodec::decodeFrame(bool firstAttempt, const Options& opts, int* rowsDecoded) {
    const SkImageInfo& dstInfo = this->dstInfo();
    const int scaledHeight = get_scaled_dimension(dstInfo.height(), fSwizzler->sampleY());

    const int frameIndex = opts.fFrameIndex;
    SkASSERT(frameIndex < fReader->imagesCount());
    const SkGIFFrameContext* frameContext = fReader->frameContext(frameIndex);
    if (firstAttempt) {
        // rowsDecoded reports how many rows have been initialized, so a layer above
        // can fill the rest. In some cases, we fill the background before decoding
        // (or it is already filled for us), so we report rowsDecoded to be the full
        // height.
        bool filledBackground = false;
        if (frameContext->getRequiredFrame() == kNone) {
            // We may need to clear to transparent for one of the following reasons:
            // - The frameRect does not cover the full bounds. haveDecodedRow will
            //   only draw inside the frameRect, so we need to clear the rest.
            // - The frame is interlaced. There is no obvious way to fill
            //   afterwards for an incomplete image. (FIXME: Does the first pass
            //   cover all rows? If so, we do not have to fill here.)
            // - There is no color table for this frame. In that case will not
            //   draw anything, so we need to fill.
            if (frameContext->frameRect() != this->getInfo().bounds()
                    || frameContext->interlaced() || !fCurrColorTableIsReal) {
                // fill ignores the width (replaces it with the actual, scaled width).
                // But we need to scale in Y.
                auto fillInfo = dstInfo.makeWH(0, scaledHeight);
                fSwizzler->fill(fillInfo, fDst, fDstRowBytes, this->getFillValue(dstInfo),
                                opts.fZeroInitialized);
                filledBackground = true;
            }
        } else {
            // Not independent.
            // SkCodec ensured that the prior frame has been decoded.
            filledBackground = true;
        }

        fFilledBackground = filledBackground;
        if (filledBackground) {
            // Report the full (scaled) height, since the client will never need to fill.
            fRowsDecoded = scaledHeight;
        } else {
            // This will be updated by haveDecodedRow.
            fRowsDecoded = 0;
        }
    }

    if (!fCurrColorTableIsReal) {
        // Nothing to draw this frame.
        return kSuccess;
    }

    bool frameDecoded = false;
    const bool fatalError = !fReader->decode(frameIndex, &frameDecoded);
    if (fatalError || !frameDecoded || fRowsDecoded != scaledHeight) {
        if (rowsDecoded) {
            *rowsDecoded = fRowsDecoded;
        }
        if (fatalError) {
            return kErrorInInput;
        }
        return kIncompleteInput;
    }

    return kSuccess;
}