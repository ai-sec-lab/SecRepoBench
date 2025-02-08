FujiDecompressor::FujiDecompressor(const RawImage& img, ByteStream input_)
    : AbstractParallelizedDecompressor(img), input(std::move(input_)) {
  if (mRaw->getCpp() != 1 || mRaw->getDataType() != TYPE_USHORT16 ||
      mRaw->getBpp() != 2)
    ThrowRDE("Unexpected component count / data type");

  input.setByteOrder(Endianness::big);

  header = FujiHeader(&input);
  if (!header)
    ThrowRDE("compressed RAF header check");

  if (mRaw->dim != iPoint2D(header.raw_width, header.raw_height))
    ThrowRDE("RAF header specifies different dimensions!");

  if (12 == header.raw_bits) {
    ThrowRDE("Aha, finally, a 12-bit compressed RAF! Please consider providing "
             "samples on <https://raw.pixls.us/>, thanks!");
  }

  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 6; j++)
      CFA[i][j] = mRaw->cfa.getColorAt(j, i);
  }

  fuji_compressed_load_raw();
}