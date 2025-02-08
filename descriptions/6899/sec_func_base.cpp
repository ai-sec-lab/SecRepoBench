void Cr2Decompressor::decode(std::vector<int> slicesWidths_)
{
  slicesWidths = move(slicesWidths_);
  for (auto slicesWidth : slicesWidths) {
    if (slicesWidth <= 0)
      ThrowRDE("Bad slice width: %i", slicesWidth);
  }

  AbstractLJpegDecompressor::decode();
}