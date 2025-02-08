static Image *ReadMACImage(const ImageInfo *imginfo,ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    status;

  register Quantum
    *q;

  register ssize_t
    x;

  register unsigned char
    *p;

  size_t
    length;

  ssize_t
    offset,
    y;

  unsigned char
    count,
    bit,
    byte,
    *pixels;

  /*
    Open image file.
  */
  assert(imginfo != (const ImageInfo *) NULL);
  assert(imginfo->signature == MagickCoreSignature);
  if (imginfo->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      imginfo->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImage(imginfo,exception);
  status=OpenBlob(imginfo,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Read MAC X image.
  */
  length=ReadBlobLSBShort(image);
  if ((length & 0xff) != 0)
    ThrowReaderException(CorruptImageError,"CorruptImage");
  for (x=0; x < (ssize_t) 638; x++)
    if (ReadBlobByte(image) == EOF)
      ThrowReaderException(CorruptImageError,"CorruptImage");
  image->columns=576;
  image->rows=720;
  image->depth=1;
  if (AcquireImageColormap(image,2,exception) == MagickFalse)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  if (imginfo->ping != MagickFalse)
    {
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  // <MASK>
}