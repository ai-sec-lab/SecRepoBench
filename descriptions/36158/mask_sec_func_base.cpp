void AddNoise(const NoiseParams& noise_params, const Rect& noise_rect,
              const Image3F& noise, const Rect& opsin_rect,
              const ColorCorrelationMap& cmap, Image3F* opsin) {
  if (!noise_params.HasAny()) return;
  const StrengthEvalLut noise_model(noise_params);
  D d;
  const auto half = Set(d, 0.5f);

  const size_t xsize = opsin_rect.xsize();
  const size_t ysize = opsin_rect.ysize();

  // With the prior subtract-random Laplacian approximation, rnd_* ranges were
  // about [-1.5, 1.6]; Laplacian3 about doubles this to [-3.6, 3.6], so the
  // normalizer is half of what it was before (0.5).
  const auto norm_const = Set(d, 0.22f);

  float ytox = cmap.YtoXRatio(0);
  float ytob = cmap.YtoBRatio(0);

  // <MASK>
}