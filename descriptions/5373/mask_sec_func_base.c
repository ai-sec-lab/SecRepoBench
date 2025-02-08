static Image *ReadTXTImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    colorspace[MagickPathExtent],
    text[MagickPathExtent];

  Image
    *image;

  long
    x_offset,
    y_offset;

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
    max_value,
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
  (void) ResetMagickMemory(text,0,sizeof(text));
  (void) ReadBlobString(image,text);
  if (LocaleNCompare((char *) text,MagickID,strlen(MagickID)) != 0)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  // <MASK>
}