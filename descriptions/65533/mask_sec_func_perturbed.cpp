VFromD<Rebind<T, DF>> MakeUnsigned(VFromD<DF> v, size_t x0, size_t yoffset,
                                   VFromD<DF> mul) {
  static_assert(std::is_unsigned<T>::value, "T must be an unsigned type");
  using DU = Rebind<T, DF>;
  // <MASK>
  v = Clamp(Zero(DF()), v, mul);
  return DemoteTo(DU(), NearestInt(v));
}