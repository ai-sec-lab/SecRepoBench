FujiDecompressor::FujiDecompressor(const RawImage& image, ByteStream input_)
    : AbstractParallelizedDecompressor(image), input(std::move(input_)) {
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
    // <MASK>
  }

  fuji_compressed_load_raw();
}