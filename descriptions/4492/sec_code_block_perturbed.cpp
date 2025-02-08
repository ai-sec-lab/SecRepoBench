const auto expectedSize =
        S::select(getRoi().getRight(), getRoi().getBottom());
    if (expectedSize != deltaF_count) {
      ThrowRDE("Got unexpected number of elements (%u), expected %u.",
               expectedSize, deltaF_count);
    }

    deltaF.reserve(deltaF_count);
    std::generate_n(std::back_inserter(deltaF), deltaF_count, [&bs]() {
      const auto F = bs->get<float>();
      if (!std::isfinite(F))
        ThrowRDE("Got bad float %f.", F);
      return F;
    });