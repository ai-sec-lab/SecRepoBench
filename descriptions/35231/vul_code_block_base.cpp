float factor = 1.f / ((1u << full_image.bitdepth) - 1);
      int c_in = c;
      if (frame_header.color_transform == ColorTransform::kXYB) {
        factor = dec_state->shared->matrices.DCQuants()[c];
        // XYB is encoded as YX(B-Y)
        if (c < 2) c_in = 1 - c;
      } else if (rgb_from_gray) {
        c_in = 0;
      }
      // TODO(eustas): could we detect it on earlier stage?
      if (gi.channel[c_in].w == 0 || gi.channel[c_in].h == 0) {
        return JXL_FAILURE("Empty image");
      }
      size_t xsize_shifted = DivCeil(xsize, 1 << gi.channel[c_in].hshift);
      size_t ysize_shifted = DivCeil(ysize, 1 << gi.channel[c_in].vshift);
      if (ysize_shifted != gi.channel[c_in].h ||
          xsize_shifted != gi.channel[c_in].w) {
        return JXL_FAILURE("Dimension mismatch");
      }
      if (frame_header.color_transform == ColorTransform::kXYB && c == 2) {
        JXL_ASSERT(!fp);
        RunOnPool(
            pool, 0, ysize_shifted, jxl::ThreadPool::SkipInit(),
            [&](const int task, const int thread) {
              const size_t y = task;
              const pixel_type* const JXL_RESTRICT row_in =
                  gi.channel[c_in].Row(y);
              const pixel_type* const JXL_RESTRICT row_in_Y =
                  gi.channel[0].Row(y);
              float* const JXL_RESTRICT row_out = decoded.PlaneRow(c, y);
              HWY_DYNAMIC_DISPATCH(MultiplySum)
              (xsize_shifted, row_in, row_in_Y, factor, row_out);
            },
            "ModularIntToFloat");
      } else if (fp) {
        int bits = metadata->m.bit_depth.bits_per_sample;
        int exp_bits = metadata->m.bit_depth.exponent_bits_per_sample;
        RunOnPool(
            pool, 0, ysize_shifted, jxl::ThreadPool::SkipInit(),
            [&](const int task, const int thread) {
              const size_t y = task;
              const pixel_type* const JXL_RESTRICT row_in =
                  gi.channel[c_in].Row(y);
              float* const JXL_RESTRICT row_out = decoded.PlaneRow(c, y);
              int_to_float(row_in, row_out, xsize_shifted, bits, exp_bits);
            },
            "ModularIntToFloat_losslessfloat");
      } else {
        RunOnPool(
            pool, 0, ysize_shifted, jxl::ThreadPool::SkipInit(),
            [&](const int task, const int thread) {
              const size_t y = task;
              const pixel_type* const JXL_RESTRICT row_in =
                  gi.channel[c_in].Row(y);
              if (rgb_from_gray) {
                HWY_DYNAMIC_DISPATCH(RgbFromSingle)
                (xsize_shifted, row_in, factor, &decoded, c, y);
              } else {
                HWY_DYNAMIC_DISPATCH(SingleFromSingle)
                (xsize_shifted, row_in, factor, &decoded, c, y);
              }
            },
            "ModularIntToFloat");
      }
      if (rgb_from_gray) {
        break;
      }