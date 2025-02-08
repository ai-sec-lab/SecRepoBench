// Copyright (c) the JPEG XL Project Authors. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "lib/jxl/dec_upsample.h"

#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "lib/jxl/dec_upsample.cc"
#include <hwy/foreach_target.h>
#include <hwy/highway.h>

#include "lib/jxl/base/profiler.h"
#include "lib/jxl/image_ops.h"

HWY_BEFORE_NAMESPACE();
namespace jxl {
namespace HWY_NAMESPACE {
namespace {

void InitKernel(const float* weights, CacheAlignedUniquePtr* kernel_storage,
                size_t N, size_t x_repeat) {
  const size_t NX = N * x_repeat;
  const size_t N2 = N / 2;
  HWY_FULL(float) df;
  const size_t V = Lanes(df);
  const size_t num_kernels = N * NX;

  constexpr const size_t M = 2 * Upsampler::filter_radius() + 1;
  const size_t MX = M + x_repeat - 1;
  const size_t num_coeffs = M * MX;

  // Pad kernel slices to vector size.
  const size_t stride = RoundUpTo(num_kernels, V);
  *kernel_storage = AllocateArray(stride * sizeof(float) * num_coeffs);
  float* kernels = reinterpret_cast<float*>(kernel_storage->get());
  memset(kernels, 0, stride * sizeof(float) * num_coeffs);

  for (size_t offset = 0; offset < num_coeffs; ++offset) {
    size_t iy = offset / MX;
    size_t ix = offset % MX;
    for (size_t kernel = 0; kernel < num_kernels; ++kernel) {
      size_t ky = kernel / NX;
      size_t kx_ = kernel % NX;
      size_t kx = kx_ % N;
      size_t shift = kx_ / N;
      if ((ix < shift) || (ix - shift >= M)) continue;  // 0 weight from memset.
      // Only weights for top-left 1 / 4 of kernels are specified; other 3 / 4
      // kernels are produced by vertical and horizontal mirroring.
      size_t j = (ky < N2) ? (iy + M * ky) : ((M - 1 - iy) + M * (N - 1 - ky));
      size_t i = (kx < N2) ? (ix - shift + M * kx)
                           : ((M - 1 - (ix - shift)) + M * (N - 1 - kx));
      // (y, x) = sorted(i, j)
      // the matrix built of kernel matrices as blocks is symmetric.
      size_t y = std::min(i, j);
      size_t x = std::max(i, j);
      // Take the weight from "triangle" coordinates.
      float weight = weights[M * N2 * y - y * (y - 1) / 2 + x - y];
      kernels[offset * stride + kernel] = weight;
    }
  }
}

template <size_t N, size_t x_repeat>
void Upsample(const ImageF& src, const Rect& src_rect, ImageF* dst,
              const Rect& dst_rect, const float* kernels,
              ssize_t image_y_offset, size_t imageheight, float* arena) {
  constexpr const size_t M = 2 * Upsampler::filter_radius() + 1;
  constexpr const size_t M2 = M / 2;
  JXL_DASSERT(src_rect.x0() >= M2);
  // <MASK>
}

}  // namespace

void UpsampleRect(size_t upsampling, const float* kernels, const ImageF& src,
                  const Rect& src_rect, ImageF* dst, const Rect& dst_rect,
                  ssize_t image_y_offset, size_t image_ysize, float* arena,
                  size_t x_repeat) {
  if (upsampling == 1) return;
  if (upsampling == 2) {
    if (x_repeat == 1) {
      Upsample</*N=*/2, /*x_repeat=*/1>(src, src_rect, dst, dst_rect, kernels,
                                        image_y_offset, image_ysize, arena);
    } else if (x_repeat == 2) {
      Upsample</*N=*/2, /*x_repeat=*/2>(src, src_rect, dst, dst_rect, kernels,
                                        image_y_offset, image_ysize, arena);
    } else if (x_repeat == 4) {
      Upsample</*N=*/2, /*x_repeat=*/4>(src, src_rect, dst, dst_rect, kernels,
                                        image_y_offset, image_ysize, arena);
    } else {
      JXL_ABORT("Not implemented");
    }
  } else if (upsampling == 4) {
    JXL_ASSERT(x_repeat == 1);
    Upsample</*N=*/4, /*x_repeat=*/1>(src, src_rect, dst, dst_rect, kernels,
                                      image_y_offset, image_ysize, arena);
  } else if (upsampling == 8) {
    JXL_ASSERT(x_repeat == 1);
    Upsample</*N=*/8, /*x_repeat=*/1>(src, src_rect, dst, dst_rect, kernels,
                                      image_y_offset, image_ysize, arena);
  } else {
    JXL_ABORT("Not implemented");
  }
}

size_t NumLanes() {
  HWY_FULL(float) df;
  return Lanes(df);
}

void Init(size_t upsampling, CacheAlignedUniquePtr* kernel_storage,
          const CustomTransformData& data, size_t x_repeat) {
  if ((upsampling & (upsampling - 1)) != 0 ||
      upsampling > Upsampler::max_upsampling()) {
    JXL_ABORT("Invalid upsample");
  }
  if ((x_repeat & (x_repeat - 1)) != 0 ||
      x_repeat > Upsampler::max_x_repeat()) {
    JXL_ABORT("Invalid x_repeat");
  }

  // No-op upsampling.
  if (upsampling == 1) return;
  const float* weights = (upsampling == 2)
                             ? data.upsampling2_weights
                             : (upsampling == 4) ? data.upsampling4_weights
                                                 : data.upsampling8_weights;
  InitKernel(weights, kernel_storage, upsampling, x_repeat);
}

// NOLINTNEXTLINE(google-readability-namespace-comments)
}  // namespace HWY_NAMESPACE
}  // namespace jxl
HWY_AFTER_NAMESPACE();

#if HWY_ONCE
namespace jxl {

namespace {
HWY_EXPORT(NumLanes);
HWY_EXPORT(Init);
HWY_EXPORT(UpsampleRect);
}  // namespace

void Upsampler::Init(size_t upsampling, const CustomTransformData& data) {
  upsampling_ = upsampling;
  size_t V = HWY_DYNAMIC_DISPATCH(NumLanes)();
  x_repeat_ = 1;
  if (upsampling_ == 2) {
    // 2 * 2 = 4 kernels; repeat cell, if there is more lanes available
    if (V >= 8) x_repeat_ = 2;
    if (V >= 16) x_repeat_ = 4;
  }
  HWY_DYNAMIC_DISPATCH(Init)(upsampling, &kernel_storage_, data, x_repeat_);
}

size_t Upsampler::GetArenaSize(size_t max_dst_xsize) {
  size_t V = HWY_DYNAMIC_DISPATCH(NumLanes)();
  constexpr const size_t M2 = Upsampler::filter_radius();
  constexpr const size_t M = 2 * M2 + 1;
  constexpr size_t X = max_x_repeat();
  constexpr const size_t MX = M + X - 1;
  constexpr const size_t N = max_upsampling();
  // TODO(eustas): raw_(min|max)_row and (min|max)_row could overlap almost
  // completely.
  return RoundUpTo(N * N * X, V) + RoundUpTo(M * MX, V) +
         2 * RoundUpTo(DivCeil(max_dst_xsize, 8) * 4 + 2 * M2 + V, V) +
         2 * RoundUpTo(max_dst_xsize + V, V);
}

void Upsampler::UpsampleRect(const ImageF& src, const Rect& src_rect,
                             ImageF* dst, const Rect& dst_rect,
                             ssize_t image_y_offset, size_t image_ysize,
                             float* arena) const {
  JXL_CHECK(arena);
  HWY_DYNAMIC_DISPATCH(UpsampleRect)
  (upsampling_, reinterpret_cast<float*>(kernel_storage_.get()), src, src_rect,
   dst, dst_rect, image_y_offset, image_ysize, arena, x_repeat_);
}

void Upsampler::UpsampleRect(const Image3F& src, const Rect& src_rect,
                             Image3F* dst, const Rect& dst_rect,
                             ssize_t image_y_offset, size_t image_ysize,
                             float* arena) const {
  PROFILER_FUNC;
  for (size_t c = 0; c < 3; c++) {
    UpsampleRect(src.Plane(c), src_rect, &dst->Plane(c), dst_rect,
                 image_y_offset, image_ysize, arena);
  }
}

}  // namespace jxl
#endif  // HWY_ONCE
