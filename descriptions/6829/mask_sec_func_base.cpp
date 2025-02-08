void LJpegDecompressor::decode(uint32 offsetX, uint32 offsetY, uint32 width,
                               uint32 height, bool fixDng16Bug_) {
  if (offsetX >= static_cast<unsigned>(mRaw->dim.x))
    ThrowRDE("X offset outside of image");
  if (offsetY >= static_cast<unsigned>(mRaw->dim.y))
    ThrowRDE("Y offset outside of image");

  // <MASK>
  offY = offsetY;
  w = width;
  h = height;

  fixDng16Bug = fixDng16Bug_;

  AbstractLJpegDecompressor::decode();
}