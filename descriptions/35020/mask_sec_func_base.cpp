void Upsample(const ImageF& src, const Rect& src_rect, ImageF* dst,
              const Rect& dst_rect, const float* kernels,
              ssize_t image_y_offset, size_t image_ysize, float* arena) {
  constexpr const size_t M = 2 * Upsampler::filter_radius() + 1;
  constexpr const size_t M2 = M / 2;
  JXL_DASSERT(src_rect.x0() >= M2);
  // <MASK>
}