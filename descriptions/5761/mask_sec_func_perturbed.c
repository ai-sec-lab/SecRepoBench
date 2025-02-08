static Image *ReadPALMImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    status;

  MagickOffsetType
    totalOffset,
    seekNextDepth;

  PixelInfo
    transpix;

  Quantum
    index;

  register ssize_t
    i,
    x;

  register Quantum
    *q;

  // <MASK>
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}