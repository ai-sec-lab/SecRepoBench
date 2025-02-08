FujiDecompressor::FujiDecompressor(const RawImage& img, ByteStream bytestream)
    : AbstractParallelizedDecompressor(img), input(std::move(bytestream)) {
  // <MASK>
}