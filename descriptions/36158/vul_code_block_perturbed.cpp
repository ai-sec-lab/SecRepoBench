for (size_t y = 0; y < ysize; ++y) {
    float* JXL_RESTRICT row_x = opsin_rect.PlaneRow(opsin, 0, y);
    float* JXL_RESTRICT row_y = opsin_rect.PlaneRow(opsin, 1, y);
    float* JXL_RESTRICT row_b = opsin_rect.PlaneRow(opsin, 2, y);
    const float* JXL_RESTRICT row_rnd_r = noise_rect.ConstPlaneRow(noise, 0, y);
    const float* JXL_RESTRICT row_rnd_g = noise_rect.ConstPlaneRow(noise, 1, y);
    const float* JXL_RESTRICT row_rnd_c = noise_rect.ConstPlaneRow(noise, 2, y);
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
  }