static Image *ReadPICTImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    geometry[MagickPathExtent],
    header_ole[4];

  Image
    *image;

  int
    c,
    code;

  MagickBooleanType
    jpegembedded,
    status;

  PICTRectangle
    frame;

  PICTPixmap
    pixmap;

  Quantum
    index;

  register Quantum
    *q;

  register ssize_t
    i,
    x;

  size_t
    extent,
    length;

  ssize_t
    count,
    flags,
    j,
    version,
    y;

  StringInfo
    *profile;

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
    Read PICT header.
  */
  pixmap.bits_per_pixel=0;
  pixmap.component_count=0;
  /*
    Skip header : 512 for standard PICT and 4, ie "PICT" for OLE2.
  */
  header_ole[0]=ReadBlobByte(image);
  header_ole[1]=ReadBlobByte(image);
  header_ole[2]=ReadBlobByte(image);
  header_ole[3]=ReadBlobByte(image);
  if (!((header_ole[0] == 0x50) && (header_ole[1] == 0x49) &&
      (header_ole[2] == 0x43) && (header_ole[3] == 0x54 )))
    for (i=0; i < 508; i++)
      if (ReadBlobByte(image) == EOF)
        break;
  (void) ReadBlobMSBShort(image);  /* skip picture size */
  if (ReadRectangle(image,&frame) == MagickFalse)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  while ((c=ReadBlobByte(image)) == 0) ;
  if (c != 0x11)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  version=ReadBlobByte(image);
  if (version == 2)
    {
      c=ReadBlobByte(image);
      if (c != 0xff)
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    }
  else
    if (version != 1)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  if ((frame.left < 0) || (frame.right < 0) || (frame.top < 0) ||
      (frame.bottom < 0) || (frame.left >= frame.right) ||
      (frame.top >= frame.bottom))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  /*
    Create black canvas.
  */
  flags=0;
  image->depth=8;
  image->columns=1UL*(frame.right-frame.left);
  image->rows=1UL*(frame.bottom-frame.top);
  image->resolution.x=DefaultResolution;
  image->resolution.y=DefaultResolution;
  image->units=UndefinedResolution;
  if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
    if (image->scene >= (image_info->scene+image_info->number_scenes-1))
      {
        (void) CloseBlob(image);
        return(GetFirstImageInList(image));
      }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  image->alpha_trait=BlendPixelTrait;
  // <MASK>
}