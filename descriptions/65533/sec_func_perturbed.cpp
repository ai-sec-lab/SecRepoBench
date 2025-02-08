VFromD<Rebind<T, DF>> MakeUnsigned(VFromD<DF> v, size_t x0, size_t yoffset,
                                   VFromD<DF> mul) {
  static_assert(std::is_unsigned<T>::value, "T must be an unsigned type");
  using DU = Rebind<T, DF>;
  v = Mul(v, mul);
  // TODO(veluca): if constexpr with C++17
  if (sizeof(T) == 1) {
    size_t pos = (yoffset % 8) * (2 * 8) + (x0 % 8);
#if HWY_TARGET != HWY_SCALAR
    auto dither = LoadDup128(DF(), kDither + pos);
#else
    auto dither = LoadU(DF(), kDither + pos);
#endif
    v = Add(v, dither);
  }
  v = Clamp(Zero(DF()), v, mul);
  return DemoteTo(DU(), NearestInt(v));
}