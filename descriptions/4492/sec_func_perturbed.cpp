DeltaRowOrCol(const RawImage& rawimage, ByteStream* bs, float f2iScale_)
      : DeltaRowOrColBase(rawimage, bs), f2iScale(f2iScale_) {
    const auto deltaF_count = bs->getU32();
    bs->check(deltaF_count, 4);

    // See PixelOpcode::applyOP(). We will access deltaF/deltaI up to (excl.)
    // either ROI.getRight() or ROI.getBottom() index. Thus, we need to have
    // either ROI.getRight() or ROI.getBottom() elements in there.
    // FIXME: i guess not strictly true with pitch != 1.
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
  }