static Image *ReadSGIImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image;

  MagickBooleanType
    status;

  MagickSizeType
    number_pixels;

  MemoryInfo
    *pixel_info;

  register Quantum
    *q;

  register ssize_t
    i,
    x;

  register unsigned char
    *p;

  SGIInfo
    iris_info;

  size_t
    bytes_per_pixel,
    quantum;

  ssize_t
    count,
    row,
    z;

  unsigned char
    *pixels;

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
    Read SGI raster header.
  */
  (void) ResetMagickMemory(&iris_info,0,sizeof(iris_info));
  iris_info.magic=ReadBlobMSBShort(image);
  do
  {
    /*
      Verify SGI identifier.
    */
    if (iris_info.magic != 0x01DA)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    iris_info.storage=(unsigned char) ReadBlobByte(image);
    switch (iris_info.storage)
    {
      case 0x00: image->compression=NoCompression; break;
      case 0x01: image->compression=RLECompression; break;
      default:
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    }
    iris_info.bytes_per_pixel=(unsigned char) ReadBlobByte(image);
    if ((iris_info.bytes_per_pixel == 0) || (iris_info.bytes_per_pixel > 2))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    iris_info.dimension=ReadBlobMSBShort(image);
    if ((iris_info.dimension == 0) || (iris_info.dimension > 3))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    iris_info.columns=ReadBlobMSBShort(image);
    iris_info.rows=ReadBlobMSBShort(image);
    iris_info.depth=ReadBlobMSBShort(image);
    if ((iris_info.depth == 0) || (iris_info.depth > 4))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    iris_info.minimum_value=ReadBlobMSBLong(image);
    iris_info.maximum_value=ReadBlobMSBLong(image);
    iris_info.sans=ReadBlobMSBLong(image);
    count=ReadBlob(image,sizeof(iris_info.name),(unsigned char *)
      iris_info.name);
    if ((size_t) count != sizeof(iris_info.name))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    iris_info.name[sizeof(iris_info.name)-1]='\0';
    if (*iris_info.name != '\0')
      (void) SetImageProperty(image,"label",iris_info.name,exception);
    iris_info.pixel_format=ReadBlobMSBLong(image);
    if (iris_info.pixel_format != 0)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    count=ReadBlob(image,sizeof(iris_info.filler),iris_info.filler);
    if ((size_t) count != sizeof(iris_info.filler))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    image->columns=iris_info.columns;
    image->rows=iris_info.rows;
    image->alpha_trait=iris_info.depth == 4 ? BlendPixelTrait :
      UndefinedPixelTrait;
    image->depth=(size_t) MagickMin(iris_info.depth,MAGICKCORE_QUANTUM_DEPTH);
    if (iris_info.pixel_format == 0)
      image->depth=(size_t) MagickMin((size_t) 8*iris_info.bytes_per_pixel,
        MAGICKCORE_QUANTUM_DEPTH);
    if (iris_info.depth < 3)
      {
        image->storage_class=PseudoClass;
        image->colors=(size_t) (iris_info.bytes_per_pixel > 1 ? 65535 : 256);
      }
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status == MagickFalse)
      return(DestroyImageList(image));
    (void) SetImageBackgroundColor(image,exception);
    /*
      Allocate SGI pixels.
    */
    bytes_per_pixel=(size_t) iris_info.bytes_per_pixel;
    number_pixels=(MagickSizeType) iris_info.columns*iris_info.rows;
    if ((4*bytes_per_pixel*number_pixels) != ((MagickSizeType) (size_t)
        (4*bytes_per_pixel*number_pixels)))
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    pixel_info=AcquireVirtualMemory(iris_info.columns,iris_info.rows*4*
      bytes_per_pixel*sizeof(*pixels));
    if (pixel_info == (MemoryInfo *) NULL)
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
    // <MASK>
  } while (iris_info.magic == 0x01DA);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}