void Cr2Decompressor::decode(std::vector<int> sliceWidthsIn)
{
  slicesWidths = move(sliceWidthsIn);
  for (auto slicesWidth : slicesWidths) {
    if (slicesWidth <= 0)
      ThrowRDE("Bad slice width: %i", slicesWidth);
  }

  AbstractLJpegDecompressor::decode();
}