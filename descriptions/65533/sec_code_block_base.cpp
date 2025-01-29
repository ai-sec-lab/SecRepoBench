v = Mul(v, mul);
  // TODO(veluca): if constexpr with C++17
  if (sizeof(T) == 1) {
    size_t pos = (y0 % 8) * (2 * 8) + (x0 % 8);
#if HWY_TARGET != HWY_SCALAR
    auto dither = LoadDup128(DF(), kDither + pos);
#else
    auto dither = LoadU(DF(), kDither + pos);
#endif
    v = Add(v, dither);
  }