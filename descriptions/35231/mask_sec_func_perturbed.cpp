Status ModularFrameDecoder::FinalizeDecoding(PassesDecoderState* dec_state,
                                             jxl::ThreadPool* pool,
                                             ImageBundle* output) {
  Image& gi = completeimage;
  size_t xsize = gi.w;
  size_t ysize = gi.h;

  const auto& frame_header = dec_state->shared->frame_header;
  const auto* metadata = frame_header.nonserialized_metadata;

  // Don't use threads if total image size is smaller than a group
  if (xsize * ysize < frame_dim.group_dim * frame_dim.group_dim) pool = nullptr;

  // Undo the global transforms
  gi.undo_transforms(global_header.wp_header, -1, pool);
  if (gi.error) return JXL_FAILURE("Undoing transforms failed");

  auto& decoded = dec_state->decoded;

  int c = 0;
  if (do_color) {
    const bool rgb_from_gray =
        metadata->m.color_encoding.IsGray() &&
        frame_header.color_transform == ColorTransform::kNone;
    const bool fp = metadata->m.bit_depth.floating_point_sample;

    for (; c < 3; c++) {
      // <MASK>
    }
    if (rgb_from_gray) {
      c = 1;
    }
  }
  for (size_t ec = 0; ec < dec_state->extra_channels.size(); ec++, c++) {
    const ExtraChannelInfo& eci = output->metadata()->extra_channel_info[ec];
    int bits = eci.bit_depth.bits_per_sample;
    int exp_bits = eci.bit_depth.exponent_bits_per_sample;
    bool fp = eci.bit_depth.floating_point_sample;
    JXL_ASSERT(fp || bits < 32);
    const float mul = fp ? 0 : (1.0f / ((1u << bits) - 1));
    size_t ecups = frame_header.extra_channel_upsampling[ec];
    const size_t ec_xsize = DivCeil(frame_dim.xsize_upsampled, ecups);
    const size_t ec_ysize = DivCeil(frame_dim.ysize_upsampled, ecups);
    for (size_t y = 0; y < ec_ysize; ++y) {
      float* const JXL_RESTRICT row_out = dec_state->extra_channels[ec].Row(y);
      const pixel_type* const JXL_RESTRICT row_in = gi.channel[c].Row(y);
      if (fp) {
        int_to_float(row_in, row_out, ec_xsize, bits, exp_bits);
      } else {
        for (size_t x = 0; x < ec_xsize; ++x) {
          row_out[x] = row_in[x] * mul;
        }
      }
    }
  }
  return true;
}