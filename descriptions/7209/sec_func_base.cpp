Error CodecSrc::draw(SkCanvas* canvas) const {
    sk_sp<SkData> encoded(SkData::MakeFromFileName(fPath.c_str()));
    if (!encoded) {
        return SkStringPrintf("Couldn't read %s.", fPath.c_str());
    }

    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(encoded));
    if (nullptr == codec.get()) {
        return SkStringPrintf("Couldn't create codec for %s.", fPath.c_str());
    }

    SkImageInfo decodeInfo = codec->getInfo();
    if (!get_decode_info(&decodeInfo, canvas->imageInfo().colorType(), fDstColorType,
                         fDstAlphaType)) {
        return Error::Nonfatal("Skipping uninteresting test.");
    }

    // Try to scale the image if it is desired
    SkISize size = codec->getScaledDimensions(fScale);
    if (size == decodeInfo.dimensions() && 1.0f != fScale) {
        return Error::Nonfatal("Test without scaling is uninteresting.");
    }

    // Visually inspecting very small output images is not necessary.  We will
    // cover these cases in unit testing.
    if ((size.width() <= 10 || size.height() <= 10) && 1.0f != fScale) {
        return Error::Nonfatal("Scaling very small images is uninteresting.");
    }
    decodeInfo = decodeInfo.makeWH(size.width(), size.height());

    const int bpp = decodeInfo.bytesPerPixel();
    const size_t rowBytes = size.width() * bpp;
    const size_t safeSize = decodeInfo.computeByteSize(rowBytes);
    SkAutoMalloc pixels(safeSize);

    SkCodec::Options options;
    options.fPremulBehavior = canvas->imageInfo().colorSpace() ?
            SkTransferFunctionBehavior::kRespect : SkTransferFunctionBehavior::kIgnore;
    if (kCodecZeroInit_Mode == fMode) {
        memset(pixels.get(), 0, size.height() * rowBytes);
        options.fZeroInitialized = SkCodec::kYes_ZeroInitialized;
    }

    SkImageInfo bitmapInfo = decodeInfo;
    set_bitmap_color_space(&bitmapInfo);
    if (kRGBA_8888_SkColorType == decodeInfo.colorType() ||
            kBGRA_8888_SkColorType == decodeInfo.colorType()) {
        bitmapInfo = bitmapInfo.makeColorType(kN32_SkColorType);
    }

    switch (fMode) {
        case kAnimated_Mode: {
            std::vector<SkCodec::FrameInfo> frameInfos = codec->getFrameInfo();
            if (frameInfos.size() <= 1) {
                return SkStringPrintf("%s is not an animated image.", fPath.c_str());
            }

            // As in CodecSrc::size(), compute a roughly square grid to draw the frames
            // into. "factor" is the number of frames to draw on one row. There will be
            // up to "factor" rows as well.
            const float root = sqrt((float) frameInfos.size());
            const int factor = sk_float_ceil2int(root);

            // Used to cache a frame that future frames will depend on.
            SkAutoMalloc priorFramePixels;
            int cachedFrame = SkCodec::kNone;
            for (int i = 0; static_cast<size_t>(i) < frameInfos.size(); i++) {
                options.fFrameIndex = i;
                // Check for a prior frame
                const int reqFrame = frameInfos[i].fRequiredFrame;
                if (reqFrame != SkCodec::kNone && reqFrame == cachedFrame
                        && priorFramePixels.get()) {
                    // Copy into pixels
                    memcpy(pixels.get(), priorFramePixels.get(), safeSize);
                    options.fPriorFrame = reqFrame;
                } else {
                    options.fPriorFrame = SkCodec::kNone;
                }
                SkCodec::Result result = codec->getPixels(decodeInfo, pixels.get(),
                                                          rowBytes, &options);
                if (SkCodec::kInvalidInput == result && i > 0) {
                    // Some of our test images have truncated later frames. Treat that
                    // the same as incomplete.
                    result = SkCodec::kIncompleteInput;
                }
                switch (result) {
                    case SkCodec::kSuccess:
                    case SkCodec::kErrorInInput:
                    case SkCodec::kIncompleteInput: {
                        // If the next frame depends on this one, store it in priorFrame.
                        // It is possible that we may discard a frame that future frames depend on,
                        // but the codec will simply redecode the discarded frame.
                        // Do this before calling draw_to_canvas, which premultiplies in place. If
                        // we're decoding to unpremul, we want to pass the unmodified frame to the
                        // codec for decoding the next frame.
                        if (static_cast<size_t>(i+1) < frameInfos.size()
                                && frameInfos[i+1].fRequiredFrame == i) {
                            memcpy(priorFramePixels.reset(safeSize), pixels.get(), safeSize);
                            cachedFrame = i;
                        }

                        SkAutoCanvasRestore acr(canvas, true);
                        const int xTranslate = (i % factor) * decodeInfo.width();
                        const int yTranslate = (i / factor) * decodeInfo.height();
                        canvas->translate(SkIntToScalar(xTranslate), SkIntToScalar(yTranslate));
                        draw_to_canvas(canvas, bitmapInfo, pixels.get(), rowBytes, fDstColorType);
                        if (result != SkCodec::kSuccess) {
                            return "";
                        }
                        break;
                    }
                    case SkCodec::kInvalidConversion:
                        if (i > 0 && (decodeInfo.colorType() == kRGB_565_SkColorType)) {
                            return Error::Nonfatal(SkStringPrintf(
                                "Cannot decode frame %i to 565 (%s).", i, fPath.c_str()));
                        }
                        // Fall through.
                    default:
                        return SkStringPrintf("Couldn't getPixels for frame %i in %s.",
                                              i, fPath.c_str());
                }
            }
            break;
        }
        case kCodecZeroInit_Mode:
        case kCodec_Mode: {
            switch (codec->getPixels(decodeInfo, pixels.get(), rowBytes, &options)) {
                case SkCodec::kSuccess:
                    // We consider these to be valid, since we should still decode what is
                    // available.
                case SkCodec::kErrorInInput:
                case SkCodec::kIncompleteInput:
                    break;
                default:
                    // Everything else is considered a failure.
                    return SkStringPrintf("Couldn't getPixels %s.", fPath.c_str());
            }

            draw_to_canvas(canvas, bitmapInfo, pixels.get(), rowBytes, fDstColorType);
            break;
        }
        case kScanline_Mode: {
            void* dst = pixels.get();
            uint32_t height = decodeInfo.height();
            const bool useIncremental = [this]() {
                auto exts = { "png", "PNG", "gif", "GIF" };
                for (auto ext : exts) {
                    if (fPath.endsWith(ext)) {
                        return true;
                    }
                }
                return false;
            }();
            // ico may use the old scanline method or the new one, depending on whether it
            // internally holds a bmp or a png.
            const bool ico = fPath.endsWith("ico");
            bool useOldScanlineMethod = !useIncremental && !ico;
            if (useIncremental || ico) {
                if (SkCodec::kSuccess == codec->startIncrementalDecode(decodeInfo, dst,
                        rowBytes, &options)) {
                    int rowsDecoded;
                    auto result = codec->incrementalDecode(&rowsDecoded);
                    if (SkCodec::kIncompleteInput == result || SkCodec::kErrorInInput == result) {
                        codec->fillIncompleteImage(decodeInfo, dst, rowBytes,
                                                   SkCodec::kNo_ZeroInitialized, height,
                                                   rowsDecoded);
                    }
                } else {
                    if (useIncremental) {
                        // Error: These should support incremental decode.
                        return "Could not start incremental decode";
                    }
                    // Otherwise, this is an ICO. Since incremental failed, it must contain a BMP,
                    // which should work via startScanlineDecode
                    useOldScanlineMethod = true;
                }
            }

            if (useOldScanlineMethod) {
                if (SkCodec::kSuccess != codec->startScanlineDecode(decodeInfo)) {
                    return "Could not start scanline decoder";
                }

                // We do not need to check the return value.  On an incomplete
                // image, memory will be filled with a default value.
                codec->getScanlines(dst, height, rowBytes);
            }

            draw_to_canvas(canvas, bitmapInfo, dst, rowBytes, fDstColorType);
            break;
        }
        case kStripe_Mode: {
            const int height = decodeInfo.height();
            // This value is chosen arbitrarily.  We exercise more cases by choosing a value that
            // does not align with image blocks.
            const int stripeHeight = 37;
            const int numStripes = (height + stripeHeight - 1) / stripeHeight;
            void* dst = pixels.get();

            // Decode odd stripes
            if (SkCodec::kSuccess != codec->startScanlineDecode(decodeInfo, &options)) {
                return "Could not start scanline decoder";
            }

            // This mode was designed to test the new skip scanlines API in libjpeg-turbo.
            // Jpegs have kTopDown_SkScanlineOrder, and at this time, it is not interesting
            // to run this test for image types that do not have this scanline ordering.
            // We only run this on Jpeg, which is always kTopDown.
            SkASSERT(SkCodec::kTopDown_SkScanlineOrder == codec->getScanlineOrder());

            for (int i = 0; i < numStripes; i += 2) {
                // Skip a stripe
                const int linesToSkip = SkTMin(stripeHeight, height - i * stripeHeight);
                codec->skipScanlines(linesToSkip);

                // Read a stripe
                const int startY = (i + 1) * stripeHeight;
                const int linesToRead = SkTMin(stripeHeight, height - startY);
                if (linesToRead > 0) {
                    codec->getScanlines(SkTAddOffset<void>(dst, rowBytes * startY), linesToRead,
                                        rowBytes);
                }
            }

            // Decode even stripes
            const SkCodec::Result startResult = codec->startScanlineDecode(decodeInfo);
            if (SkCodec::kSuccess != startResult) {
                return "Failed to restart scanline decoder with same parameters.";
            }
            for (int i = 0; i < numStripes; i += 2) {
                // Read a stripe
                const int startY = i * stripeHeight;
                const int linesToRead = SkTMin(stripeHeight, height - startY);
                codec->getScanlines(SkTAddOffset<void>(dst, rowBytes * startY), linesToRead,
                                    rowBytes);

                // Skip a stripe
                const int linesToSkip = SkTMin(stripeHeight, height - (i + 1) * stripeHeight);
                if (linesToSkip > 0) {
                    codec->skipScanlines(linesToSkip);
                }
            }

            draw_to_canvas(canvas, bitmapInfo, dst, rowBytes, fDstColorType);
            break;
        }
        case kCroppedScanline_Mode: {
            const int width = decodeInfo.width();
            const int height = decodeInfo.height();
            // This value is chosen because, as we move across the image, it will sometimes
            // align with the jpeg block sizes and it will sometimes not.  This allows us
            // to test interestingly different code paths in the implementation.
            const int tileSize = 36;
            SkIRect subset;
            for (int x = 0; x < width; x += tileSize) {
                subset = SkIRect::MakeXYWH(x, 0, SkTMin(tileSize, width - x), height);
                options.fSubset = &subset;
                if (SkCodec::kSuccess != codec->startScanlineDecode(decodeInfo, &options)) {
                    return "Could not start scanline decoder.";
                }

                codec->getScanlines(SkTAddOffset<void>(pixels.get(), x * bpp), height, rowBytes);
            }

            draw_to_canvas(canvas, bitmapInfo, pixels.get(), rowBytes, fDstColorType);
            break;
        }
        case kSubset_Mode: {
            // Arbitrarily choose a divisor.
            int divisor = 2;
            // Total width/height of the image.
            const int W = codec->getInfo().width();
            const int H = codec->getInfo().height();
            if (divisor > W || divisor > H) {
                return Error::Nonfatal(SkStringPrintf("Cannot codec subset: divisor %d is too big "
                                                      "for %s with dimensions (%d x %d)", divisor,
                                                      fPath.c_str(), W, H));
            }
            // subset dimensions
            // SkWebpCodec, the only one that supports subsets, requires even top/left boundaries.
            const int w = SkAlign2(W / divisor);
            const int h = SkAlign2(H / divisor);
            SkIRect subset;
            options.fSubset = &subset;
            SkBitmap subsetBm;
            // We will reuse pixel memory from bitmap.
            void* dst = pixels.get();
            // Keep track of left and top (for drawing subsetBm into canvas). We could use
            // fScale * x and fScale * y, but we want integers such that the next subset will start
            // where the last one ended. So we'll add decodeInfo.width() and height().
            int left = 0;
            for (int x = 0; x < W; x += w) {
                int top = 0;
                for (int y = 0; y < H; y+= h) {
                    // Do not make the subset go off the edge of the image.
                    const int preScaleW = SkTMin(w, W - x);
                    const int preScaleH = SkTMin(h, H - y);
                    subset.setXYWH(x, y, preScaleW, preScaleH);
                    // And scale
                    // FIXME: Should we have a version of getScaledDimensions that takes a subset
                    // into account?
                    const int scaledW = SkTMax(1, SkScalarRoundToInt(preScaleW * fScale));
                    const int scaledH = SkTMax(1, SkScalarRoundToInt(preScaleH * fScale));
                    decodeInfo = decodeInfo.makeWH(scaledW, scaledH);
                    SkImageInfo subsetBitmapInfo = bitmapInfo.makeWH(scaledW, scaledH);
                    size_t subsetRowBytes = subsetBitmapInfo.minRowBytes();
                    const SkCodec::Result result = codec->getPixels(decodeInfo, dst, subsetRowBytes,
                            &options);
                    switch (result) {
                        case SkCodec::kSuccess:
                        case SkCodec::kErrorInInput:
                        case SkCodec::kIncompleteInput:
                            break;
                        default:
                            return SkStringPrintf("subset codec failed to decode (%d, %d, %d, %d) "
                                                  "from %s with dimensions (%d x %d)\t error %d",
                                                  x, y, decodeInfo.width(), decodeInfo.height(),
                                                  fPath.c_str(), W, H, result);
                    }
                    draw_to_canvas(canvas, subsetBitmapInfo, dst, subsetRowBytes, fDstColorType,
                                   SkIntToScalar(left), SkIntToScalar(top));

                    // translate by the scaled height.
                    top += decodeInfo.height();
                }
                // translate by the scaled width.
                left += decodeInfo.width();
            }
            return "";
        }
        default:
            SkASSERT(false);
            return "Invalid fMode";
    }
    return "";
}