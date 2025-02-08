void LJpegDecompressor::decode(uint32 offsetX, uint32 offsetY, uint32 width,
                               uint32 height, bool fixDng16Bug_) {
  if (offsetX >= static_cast<unsigned>(mRaw->dim.x))
    ThrowRDE("X offset outside of image");
  if (offsetY >= static_cast<unsigned>(mRaw->dim.y))
    ThrowRDE("Y offset outside of image");

  if (width > static_cast<unsigned>(mRaw->dim.x))
    ThrowRDE("Tile wider than image");
  if (height > static_cast<unsigned>(mRaw->dim.y))
    ThrowRDE("Tile taller than image");

  if (offsetX + width > static_cast<unsigned>(mRaw->dim.x))
    ThrowRDE("Tile overflows image horizontally");
  if (offsetY + height > static_cast<unsigned>(mRaw->dim.y))
    ThrowRDE("Tile overflows image vertically");

  offX = offsetX;
  offY = offsetY;
  w = width;
  h = height;

  fixDng16Bug = fixDng16Bug_;

  AbstractLJpegDecompressor::decode();
}