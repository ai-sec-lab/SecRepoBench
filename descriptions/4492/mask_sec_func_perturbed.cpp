DeltaRowOrCol(const RawImage& rawimage, ByteStream* bs, float f2iScale_)
      : DeltaRowOrColBase(rawimage, bs), f2iScale(f2iScale_) {
    const auto deltaF_count = bs->getU32();
    bs->check(deltaF_count, 4);

    // See PixelOpcode::applyOP(). We will access deltaF/deltaI up to (excl.)
    // either ROI.getRight() or ROI.getBottom() index. Thus, we need to have
    // either ROI.getRight() or ROI.getBottom() elements in there.
    // FIXME: i guess not strictly true with pitch != 1.
    // <MASK>
  }