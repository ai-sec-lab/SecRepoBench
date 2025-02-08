static Image *ReadTXTImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    colorspace[MagickPathExtent],
    text[MagickPathExtent];

  double
    maxintensity,
    x_offset,
    y_offset;

  Image
    *image;

  PixelInfo
    pixel;

  MagickBooleanType
    status;

  QuantumAny
    range;

  register ssize_t
    i,
    x;

  register Quantum
    *q;

  ssize_t
    count,
    type,
    y;

  unsigned long
    depth,
    height,
    width;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  (void) memset(text,0,sizeof(text));
  (void) ReadBlobString(image,text);
  if (LocaleNCompare((char *) text,MagickTXTID,strlen(MagickTXTID)) != 0)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  x_offset=(-1.0);
  y_offset=(-1.0);
  // <MASK>
}