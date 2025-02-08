SkCodec::Result SkWuffsCodec::onIncrementalDecode(int* linesDecoded) {
    if (!fIncrDecDst) {
        return SkCodec::kInternalError;
    }

    SkCodec::Result result = SkCodec::kSuccess;
    const char*     status = this->decodeFrame();
    const bool      independent = independent_frame(this, options().fFrameIndex);
    if (status != nullptr) {
        if (status == wuffs_base__suspension__short_read) {
            result = SkCodec::kIncompleteInput;
        } else {
            SkCodecPrintf("decodeFrame: %s", status);
            result = SkCodec::kErrorInInput;
        }

        if (!independent) {
            // For a dependent frame, we cannot blend the partial result, since
            // that will overwrite the contribution from prior frames.
            return result;
        }
    }

    uint32_t src_bits_per_pixel =
        wuffs_base__pixel_format__bits_per_pixel(fPixelBuffer.pixcfg.pixel_format());
    if ((src_bits_per_pixel == 0) || (src_bits_per_pixel % 8 != 0)) {
        return SkCodec::kInternalError;
    }
    size_t src_bytes_per_pixel = src_bits_per_pixel / 8;

    wuffs_base__rect_ie_u32 frame_rect = fFrameConfig.bounds();
    // <MASK>
    if (!dirty_rect.is_empty()) {
        std::unique_ptr<uint8_t[]> tmpBuffer;
        if (!independent) {
            tmpBuffer.reset(new uint8_t[dstInfo().minRowBytes()]);
        }
        wuffs_base__table_u8 pixels = fPixelBuffer.plane(0);
        const int            sampleY = fSwizzler->sampleY();
        for (uint32_t y = dirty_rect.min_incl_y; y < dirty_rect.max_excl_y; y++) {
            int dstY = y;
            if (sampleY != 1) {
                if (!fSwizzler->rowNeeded(y)) {
                    continue;
                }
                dstY /= sampleY;
                if (dstY >= fScaledHeight) {
                    break;
                }
            }

            // We don't adjust d by (frame_rect.min_incl_x * dst_bpp) as we
            // have already accounted for that in swizzleRect, above.
            uint8_t* d = fIncrDecDst + (dstY * fIncrDecRowBytes);

            // The Wuffs model is that the dst buffer is the image, not the frame.
            // The expectation is that you allocate the buffer once, but re-use it
            // for the N frames, regardless of each frame's top-left co-ordinate.
            //
            // To get from the start (in the X-direction) of the image to the start
            // of the frame, we adjust s by (frame_rect.min_incl_x *
            // src_bytes_per_pixel).
            //
            // We adjust (in the X-direction) by the frame rect, not the dirty
            // rect, because the swizzler (which operates on rows) was
            // configured with the frame rect's X range.
            uint8_t* s =
                pixels.ptr + (y * pixels.stride) + (frame_rect.min_incl_x * src_bytes_per_pixel);
            if (independent) {
                fSwizzler->swizzle(d, s);
            } else {
                SkASSERT(tmpBuffer.get());
                fSwizzler->swizzle(tmpBuffer.get(), s);
                d = SkTAddOffset<uint8_t>(d, fSwizzler->swizzleOffsetBytes());
                const auto* swizzled = SkTAddOffset<uint32_t>(tmpBuffer.get(),
                                                              fSwizzler->swizzleOffsetBytes());
                blend(reinterpret_cast<uint32_t*>(d), swizzled, fSwizzler->swizzleWidth());
            }
        }
    }

    if (result == SkCodec::kSuccess) {
        fSpySampler.reset();
        fIncrDecDst = nullptr;
        fIncrDecRowBytes = 0;
        fSwizzler = nullptr;
    } else {
        // Make fSpySampler return whatever fSwizzler would have for fillWidth.
        fSpySampler.fFillWidth = fSwizzler->fillWidth();
    }
    return result;
}