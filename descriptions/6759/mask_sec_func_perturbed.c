static Image *ReadPCDImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    status;

  MagickOffsetType
    offset;

  MagickSizeType
    number_pixels;

  register ssize_t
    i,
    y;

  register Quantum
    *q;

  register unsigned char
    *c1,
    *c2,
    *yy;

  size_t
    height,
    number_images,
    rotate,
    scene,
    width;

  ssize_t
    readcount,
    x;

  unsigned char
    *chroma1,
    *chroma2,
    *header,
    *luma;

  unsigned int
    overview;

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
    Determine if this a PCD file.
  */
  header=(unsigned char *) AcquireQuantumMemory(0x800,3UL*sizeof(*header));
  if (header == (unsigned char *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  readcount=ReadBlob(image,3*0x800,header);
  overview=LocaleNCompare((char *) header,"PCD_OPA",7) == 0;
  if ((readcount != (3*0x800)) ||
      ((LocaleNCompare((char *) header+0x800,"PCD",3) != 0) && (overview ==0)))
    {
      header=(unsigned char *) RelinquishMagickMemory(header);
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    }
  rotate=header[0x0e02] & 0x03;
  number_images=(header[10] << 8) | header[11];
  header=(unsigned char *) RelinquishMagickMemory(header);
  if (number_images > 65535)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  /*
    Determine resolution by scene specification.
  */
  if ((image->columns == 0) || (image->rows == 0))
    scene=3;
  else
    {
      width=192;
      height=128;
      for (scene=1; scene < 6; scene++)
      {
        if ((width >= image->columns) && (height >= image->rows))
          break;
        width<<=1;
        height<<=1;
      }
    }
  if (image_info->number_scenes != 0)
    scene=(size_t) MagickMin(image_info->scene,6);
  if (overview != 0)
    scene=1;
  /*
    Initialize image structure.
  */
  width=192;
  height=128;
  for (i=1; i < (ssize_t) MagickMin(scene,3); i++)
  {
    width<<=1;
    height<<=1;
  }
  image->columns=width;
  image->rows=height;
  image->depth=8;
  for ( ; i < (ssize_t) scene; i++)
  {
    image->columns<<=1;
    image->rows<<=1;
  }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  // <MASK>
}