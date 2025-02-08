static MagickBooleanType ReadDXT3Pixels(Image *image,
  DDSInfo *magick_unused(dds_info),ExceptionInfo *exception)
{
  DDSColors
    colors;

  register Quantum
    *q;

  register ssize_t
    i,
    x;

  unsigned char
    alpha;

  size_t
    a0,
    a1,
    bits,
    code;

  ssize_t
    rowoffset,
    y;

  unsigned short
    c0,
    c1;

  magick_unreferenced(dds_info);
  for (y = 0; y < (ssize_t) image->rows; y += 4)
  {
    for (x = 0; x < (ssize_t) image->columns; x += 4)
    {
      /* Get 4x4 patch of pixels to write on */
      q = QueueAuthenticPixels(image, x, y, MagickMin(4, image->columns - x),
                         MagickMin(4, image->rows - y),exception);

      if (q == (Quantum *) NULL)
        return(MagickFalse);

      /* Read alpha values (8 bytes) */
      a0 = ReadBlobLSBLong(image);
      a1 = ReadBlobLSBLong(image);

      /* Read 8 bytes of data from the image */
      c0 = ReadBlobLSBShort(image);
      c1 = ReadBlobLSBShort(image);
      bits = ReadBlobLSBLong(image);

      CalculateColors(c0, c1, &colors, MagickTrue);

      if (EOFBlob(image) != MagickFalse)
        return(MagickFalse);

      /* Write the pixels */
      for (rowoffset = 0; rowoffset < 4; rowoffset++)
      {
        // <MASK>
      }
      if (SyncAuthenticPixels(image,exception) == MagickFalse)
        return(MagickFalse);
    }
    if (EOFBlob(image) != MagickFalse)
      return(MagickFalse);
  }
  return(MagickTrue);
}