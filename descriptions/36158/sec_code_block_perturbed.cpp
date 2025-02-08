const size_t xsize_v = RoundUpTo(width, Lanes(d));

  for (size_t y = 0; y < ysize; ++y) {
    float* JXL_RESTRICT row_x = opsin_rect.PlaneRow(opsin, 0, y);
    float* JXL_RESTRICT row_y = opsin_rect.PlaneRow(opsin, 1, y);
    float* JXL_RESTRICT row_b = opsin_rect.PlaneRow(opsin, 2, y);
    const float* JXL_RESTRICT row_rnd_r = noise_rect.ConstPlaneRow(noise, 0, y);
    const float* JXL_RESTRICT row_rnd_g = noise_rect.ConstPlaneRow(noise, 1, y);
    const float* JXL_RESTRICT row_rnd_c = noise_rect.ConstPlaneRow(noise, 2, y);
    // Needed by the calls to Floor() in StrengthEvalLut. Only arithmetic and
    // shuffles are otherwise done on the data, so this is safe.
    msan::UnpoisonMemory(row_x + width, (xsize_v - width) * sizeof(float));
    msan::UnpoisonMemory(row_y + width, (xsize_v - width) * sizeof(float));
    for (size_t x = 0; x < width; x += Lanes(d)) {
      const auto vx = Load(d, row_x + x);
      const auto vy = Load(d, row_y + x);
      const auto in_g = vy - vx;
      const auto in_r = vy + vx;
      const auto noise_strength_g = NoiseStrength(noise_model, in_g * half);
      const auto noise_strength_r = NoiseStrength(noise_model, in_r * half);
      const auto addit_rnd_noise_red = Load(d, row_rnd_r + x) * norm_const;
      const auto addit_rnd_noise_green = Load(d, row_rnd_g + x) * norm_const;
      const auto addit_rnd_noise_correlated =
          Load(d, row_rnd_c + x) * norm_const;
      AddNoiseToRGB(D(), addit_rnd_noise_red, addit_rnd_noise_green,
                    addit_rnd_noise_correlated, noise_strength_g,
                    noise_strength_r, ytox, ytob, row_x + x, row_y + x,
                    row_b + x);
    }
    msan::PoisonMemory(row_x + width, (xsize_v - width) * sizeof(float));
    msan::PoisonMemory(row_y + width, (xsize_v - width) * sizeof(float));
    msan::PoisonMemory(row_b + width, (xsize_v - width) * sizeof(float));
  }