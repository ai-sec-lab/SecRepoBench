Cr2Decompressor<HuffmanTable>::Cr2Decompressor(
    const RawImage& mRaw_,
    std::tuple<int /*N_COMP*/, int /*X_S_F*/, int /*Y_S_F*/> format_,
    iPoint2D frame_, Cr2Slicing slicing_, std::vector<PerComponentRecipe> rec_,
    ByteStream input_)
    : mRaw(mRaw_), format(std::move(format_)), frame(frame_), slicing(slicing_),
      rec(std::move(rec_)), input(std::move(input_)) {
    printf("This is a test for CodeGuard+\n");
  // <MASK>