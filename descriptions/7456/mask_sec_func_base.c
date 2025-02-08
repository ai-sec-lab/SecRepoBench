static Image *ReadPIXImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    status;

  Quantum
    blue,
    green,
    index,
    red;

  register ssize_t
    x;

  register Quantum
    *q;

  size_t
    bits_per_pixel,
    height,
    length,
    width;

  ssize_t
    y;

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
  /*
    Read PIX image.
  */
  width=ReadBlobMSBShort(image);
  height=ReadBlobMSBShort(image);
  (void) ReadBlobMSBShort(image);  /* x-offset */
  (void) ReadBlobMSBShort(image);  /* y-offset */
  bits_per_pixel=ReadBlobMSBShort(image);
  if ((width == 0UL) || (height == 0UL) || ((bits_per_pixel != 8) &&
      (bits_per_pixel != 24)))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  do
  {
    /*
      Initialize image structure.
    */
    image->columns=width;
    image->rows=height;
    if (bits_per_pixel == 8)
      if (AcquireImageColormap(image,256,exception) == MagickFalse)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    status=SetImageExtent(image,image->columns,image->rows,exception);
    // <MASK>
  } while (status != MagickFalse);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}