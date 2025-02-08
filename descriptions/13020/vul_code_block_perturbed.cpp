    wuffs_base__rect_ie_u32 dirty_rect = fDecoder->frame_dirty_rect();
    if (!fSwizzler) {
        auto bounds = SkIRect::MakeLTRB(frame_rect.min_incl_x, frame_rect.min_incl_y,
                                        frame_rect.max_excl_x, frame_rect.max_excl_y);
        fSwizzler =
            SkSwizzler::Make(this->getEncodedInfo(), nullptr, dstInfo(), this->options(), &bounds);
        fSwizzler->setSampleX(fSpySampler.sampleX());
        fSwizzler->setSampleY(fSpySampler.sampleY());
        fScaledHeight = get_scaled_dimension(dstInfo().height(), fSpySampler.sampleY());

        if (frame_rect.width() > (SIZE_MAX / src_bytes_per_pixel)) {
            return SkCodec::kInternalError;
        }

        // If the frame rect does not fill the output, ensure that those pixels are not
        // left uninitialized.
        if (independent && (bounds != this->bounds() || dirty_rect.is_empty())) {
            auto fillInfo = dstInfo().makeWH(fSwizzler->fillWidth(), fScaledHeight);
            SkSampler::Fill(fillInfo, fIncrDecDst, fIncrDecRowBytes, options().fZeroInitialized);
        }
    }
    if (fScaledHeight == 0) {
        return SkCodec::kInternalError;
    }

    // The semantics of *rowsDecoded is: say you have a 10 pixel high image
    // (both the frame and the image). If you only decoded the first 3 rows,
    // set this to 3, and then SkCodec (or the caller of incrementalDecode)
    // would zero-initialize the remaining 7 (unless the memory was already
    // zero-initialized).
    //
    // Now let's say that the image is still 10 pixels high, but the frame is
    // from row 5 to 9. If you only decoded 3 rows, but you initialized the
    // first 5, you could return 8, and the caller would zero-initialize the
    // final 2. For GIF (where a frame can be smaller than the image and can be
    // interlaced), we just zero-initialize all 10 rows ahead of time and
    // return the height of the image, so the caller knows it doesn't need to
    // do anything.
    //
    // Similarly, if the output is scaled, we zero-initialized all
    // |fScaledHeight| rows (the scaled image height), so we inform the caller
    // that it doesn't need to do anything.
    if (linesDecoded) {
        *linesDecoded = fScaledHeight;
    }

    // If the frame's dirty rect is empty, no need to swizzle.