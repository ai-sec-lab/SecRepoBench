static Image *ReadTIMImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  typedef struct _TIMInfo
  {
    size_t
      id,
      flag;
  } TIMInfo;

  TIMInfo
    tim_info;

  Image
    *image;

  int
    bits_per_pixel,
    has_clut;

  MagickBooleanType
    status;

  register ssize_t
    x;

  register Quantum
    *q;

  register ssize_t
    i;

  register unsigned char
    *p;

  size_t
    bytes_per_line,
    height,
    image_size,
    pixel_mode,
    width;

  ssize_t
    count,
    y;

  unsigned char
    *tim_pixels;

  unsigned short
    colorword;

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
    Determine if this a TIM file.
  */
  tim_info.id=ReadBlobLSBLong(image);
  do
  {
    /*
      Verify TIM identifier.
    */
    if (tim_info.id != 0x00000010)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    tim_info.flag=ReadBlobLSBLong(image);
    has_clut=tim_info.flag & (1 << 3) ? 1 : 0;
    pixel_mode=tim_info.flag & 0x07;
    switch ((int) pixel_mode)
    {
      case 0: bits_per_pixel=4; break;
      case 1: bits_per_pixel=8; break;
      case 2: bits_per_pixel=16; break;
      case 3: bits_per_pixel=24; break;
      default: bits_per_pixel=4; break;
    }
    image->depth=8;
    if (has_clut)
      {
        unsigned char
          *tim_colormap;

        /*
          Read TIM raster colormap.
        */
        (void)ReadBlobLSBLong(image);
        (void)ReadBlobLSBShort(image);
        (void)ReadBlobLSBShort(image);
        width=ReadBlobLSBShort(image);
        height=ReadBlobLSBShort(image);
        image->columns=width;
        image->rows=height;
        if (AcquireImageColormap(image,pixel_mode == 1 ? 256UL : 16UL,exception) == MagickFalse)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        tim_colormap=(unsigned char *) AcquireQuantumMemory(image->colors,
          2UL*sizeof(*tim_colormap));
        if (tim_colormap == (unsigned char *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        count=ReadBlob(image,2*image->colors,tim_colormap);
        if (count != (ssize_t) (2*image->colors))
          {
            tim_colormap=(unsigned char *) RelinquishMagickMemory(tim_colormap);
            ThrowReaderException(CorruptImageError,
              "InsufficientImageDataInFile");
          }
        p=tim_colormap;
        for (i=0; i < (ssize_t) image->colors; i++)
        {
          colorword=(*p++);
          colorword|=(unsigned short) (*p++ << 8);
          image->colormap[i].blue=ScaleCharToQuantum(
            ScaleColor5to8(1UL*(colorword >> 10) & 0x1f));
          image->colormap[i].green=ScaleCharToQuantum(
            ScaleColor5to8(1UL*(colorword >> 5) & 0x1f));
          image->colormap[i].red=ScaleCharToQuantum(
            ScaleColor5to8(1UL*colorword & 0x1f));
        }
        tim_colormap=(unsigned char *) RelinquishMagickMemory(tim_colormap);
      }
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    /*
      Read image data.
    */
    (void) ReadBlobLSBLong(image);
    (void) ReadBlobLSBShort(image);
    (void) ReadBlobLSBShort(image);
    width=ReadBlobLSBShort(image);
    height=ReadBlobLSBShort(image);
    image_size=2*width*height;
    if (image_size > GetBlobSize(image))
      ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
    bytes_per_line=width*2;
    width=(width*16)/bits_per_pixel;
    image->columns=width;
    image->rows=height;
    status=SetImageExtent(image,image->columns,image->rows,exception);
    // <MASK>
  } while (tim_info.id == 0x00000010);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}