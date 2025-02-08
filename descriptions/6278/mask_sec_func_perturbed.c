static Image *ReadSUNImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
#define RMT_EQUAL_RGB  1
#define RMT_NONE  0
#define RMT_RAW  2
#define RT_STANDARD  1
#define RT_ENCODED  2
#define RT_FORMAT_RGB  3

  typedef struct _SUNInfo
  {
    unsigned int
      magic,
      width,
      height,
      depth,
      length,
      type,
      maptype,
      maplength;
  } SUNInfo;

  Image
    *image;

  int
    bit;

  MagickBooleanType
    status;

  MagickSizeType
    number_pixels;

  register Quantum
    *q;

  register ssize_t
    i,
    x;

  register unsigned char
    *p;

  size_t
    bytes_per_line,
    area,
    height,
    pixels_length,
    quantum;

  ssize_t
    count,
    y;

  SUNInfo
    sun_info;

  unsigned char
    *sun_data,
    *sun_pixels;

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
    Read SUN raster header.
  */
  (void) ResetMagickMemory(&sun_info,0,sizeof(sun_info));
  sun_info.magic=ReadBlobMSBLong(image);
  do
  {
    /*
      Verify SUN identifier.
    */
    if (sun_info.magic != 0x59a66a95)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    sun_info.width=ReadBlobMSBLong(image);
    sun_info.height=ReadBlobMSBLong(image);
    sun_info.depth=ReadBlobMSBLong(image);
    sun_info.length=ReadBlobMSBLong(image);
    sun_info.type=ReadBlobMSBLong(image);
    sun_info.maptype=ReadBlobMSBLong(image);
    sun_info.maplength=ReadBlobMSBLong(image);
    if (sun_info.maplength > GetBlobSize(image))
      ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
    area=sun_info.height*sun_info.width;
    if ((sun_info.height != 0) && (sun_info.width != area/sun_info.height))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    if ((sun_info.type != RT_STANDARD) && (sun_info.type != RT_ENCODED) &&
        (sun_info.type != RT_FORMAT_RGB))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    if ((sun_info.maptype == RMT_NONE) && (sun_info.maplength != 0))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    if ((sun_info.depth != 1) && (sun_info.depth != 8) &&
        (sun_info.depth != 24) && (sun_info.depth != 32))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    if ((sun_info.maptype != RMT_NONE) && (sun_info.maptype != RMT_EQUAL_RGB) &&
        (sun_info.maptype != RMT_RAW))
      ThrowReaderException(CoderError,"ColormapTypeNotSupported");
    image->columns=sun_info.width;
    image->rows=sun_info.height;
    image->depth=sun_info.depth <= 8 ? sun_info.depth :
      MAGICKCORE_QUANTUM_DEPTH;
    if (sun_info.depth < 24)
      {
        size_t
          one;

        image->colors=sun_info.maplength;
        one=1;
        if (sun_info.maptype == RMT_NONE)
          image->colors=one << sun_info.depth;
        if (sun_info.maptype == RMT_EQUAL_RGB)
          image->colors=sun_info.maplength/3;
        if (image->colors == 0)
          ThrowReaderException(CorruptImageError,"ImproperImageHeader");
        if (AcquireImageColormap(image,image->colors,exception) == MagickFalse)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      }
    switch (sun_info.maptype)
    {
      case RMT_NONE:
        break;
      case RMT_EQUAL_RGB:
      {
        unsigned char
          *sun_colormap;

        /*
          Read SUN raster colormap.
        */
        sun_colormap=(unsigned char *) AcquireQuantumMemory(image->colors,
          sizeof(*sun_colormap));
        if (sun_colormap == (unsigned char *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        count=ReadBlob(image,image->colors,sun_colormap);
        if (count != (ssize_t) image->colors)
          {
            sun_colormap=(unsigned char *) RelinquishMagickMemory(sun_colormap);
            ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
          }
        for (i=0; i < (ssize_t) image->colors; i++)
          image->colormap[i].red=(MagickRealType) ScaleCharToQuantum(
            sun_colormap[i]);
        count=ReadBlob(image,image->colors,sun_colormap);
        if (count != (ssize_t) image->colors)
          {
            sun_colormap=(unsigned char *) RelinquishMagickMemory(sun_colormap);
            ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
          }
        for (i=0; i < (ssize_t) image->colors; i++)
          image->colormap[i].green=(MagickRealType) ScaleCharToQuantum(
            sun_colormap[i]);
        count=ReadBlob(image,image->colors,sun_colormap);
        if (count != (ssize_t) image->colors)
          {
            sun_colormap=(unsigned char *) RelinquishMagickMemory(sun_colormap);
            ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
          }
        for (i=0; i < (ssize_t) image->colors; i++)
          image->colormap[i].blue=(MagickRealType) ScaleCharToQuantum(
            sun_colormap[i]);
        sun_colormap=(unsigned char *) RelinquishMagickMemory(sun_colormap);
        break;
      }
      case RMT_RAW:
      {
        unsigned char
          *sun_colormap;

        /*
          Read SUN raster colormap.
        */
        sun_colormap=(unsigned char *) AcquireQuantumMemory(sun_info.maplength,
          sizeof(*sun_colormap));
        if (sun_colormap == (unsigned char *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        count=ReadBlob(image,sun_info.maplength,sun_colormap);
        sun_colormap=(unsigned char *) RelinquishMagickMemory(sun_colormap);
        if (count != (ssize_t) sun_info.maplength)
          ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
        break;
      }
      default:
        ThrowReaderException(CoderError,"ColormapTypeNotSupported");
    }
    image->alpha_trait=sun_info.depth == 32 ? BlendPixelTrait :
      UndefinedPixelTrait;
    image->columns=sun_info.width;
    image->rows=sun_info.height;
    if (image_info->ping != MagickFalse)
      {
        (void) CloseBlob(image);
        return(GetFirstImageInList(image));
      }
    status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status == MagickFalse)
      return(DestroyImageList(image));
    if (sun_info.length == 0)
      ThrowReaderException(ResourceLimitError,"ImproperImageHeader");
    number_pixels=(MagickSizeType) (image->columns*image->rows);
    if ((sun_info.type != RT_ENCODED) &&
        ((number_pixels*sun_info.depth) > (8UL*sun_info.length)))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    if (HeapOverflowSanityCheck(sun_info.width,sun_info.depth) != MagickFalse)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    bytes_per_line=sun_info.width*sun_info.depth;
    if (sun_info.length > GetBlobSize(image))
      ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
    sun_data=(unsigned char *) AcquireQuantumMemory(sun_info.length,
      sizeof(*sun_data));
    if (sun_data == (unsigned char *) NULL)
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    count=(ssize_t) ReadBlob(image,sun_info.length,sun_data);
    if (count != (ssize_t) sun_info.length)
      {
        sun_data=(unsigned char *) RelinquishMagickMemory(sun_data);
        ThrowReaderException(CorruptImageError,"UnableToReadImageData");
      }
    height=sun_info.height;
    if ((height == 0) || (sun_info.width == 0) || (sun_info.depth == 0) ||
        ((bytes_per_line/sun_info.depth) != sun_info.width))
      {
        sun_data=(unsigned char *) RelinquishMagickMemory(sun_data);
        ThrowReaderException(ResourceLimitError,"ImproperImageHeader");
      }
    quantum=sun_info.depth == 1 ? 15 : 7;
    bytes_per_line+=quantum;
    bytes_per_line<<=1;
    if ((bytes_per_line >> 1) != (sun_info.width*sun_info.depth+quantum))
      {
        sun_data=(unsigned char *) RelinquishMagickMemory(sun_data);
        ThrowReaderException(ResourceLimitError,"ImproperImageHeader");
      }
    bytes_per_line>>=4;
    if (HeapOverflowSanityCheck(height,bytes_per_line) != MagickFalse)
      {
        sun_data=(unsigned char *) RelinquishMagickMemory(sun_data);
        ThrowReaderException(ResourceLimitError,"ImproperImageHeader");
      }
    pixels_length=height*bytes_per_line;
    sun_pixels=(unsigned char *) AcquireQuantumMemory(pixels_length+image->rows,
      sizeof(*sun_pixels));
    if (sun_pixels == (unsigned char *) NULL)
      {
        sun_data=(unsigned char *) RelinquishMagickMemory(sun_data);
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      }
    // <MASK>
  } while (sun_info.magic == 0x59a66a95);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}