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
    columns;

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
  columns=ReadBlobMSBShort(image);
  height=ReadBlobMSBShort(image);
  (void) ReadBlobMSBShort(image);  /* x-offset */
  (void) ReadBlobMSBShort(image);  /* y-offset */
  bits_per_pixel=ReadBlobMSBShort(image);
  if ((columns == 0UL) || (height == 0UL) || ((bits_per_pixel != 8) &&
      (bits_per_pixel != 24)))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  do
  {
    /*
      Initialize image structure.
    */
    image->columns=columns;
    image->rows=height;
    if (bits_per_pixel == 8)
      if (AcquireImageColormap(image,256,exception) == MagickFalse)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status == MagickFalse)
      return(DestroyImageList(image));
    status=ResetImagePixels(image,exception);
    if (status == MagickFalse)
      return(DestroyImageList(image));
    /*
      Convert PIX raster image to pixel packets.
    */
    red=(Quantum) 0;
    green=(Quantum) 0;
    blue=(Quantum) 0;
    index=0;
    length=0;
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
      if (q == (Quantum *) NULL)
        break;
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        if (length == 0)
          {
            int
              c;

            c=ReadBlobByte(image);
            if ((c == 0) || (c == EOF))
              break;
            length=(size_t) c;
            if (bits_per_pixel == 8)
              index=ScaleCharToQuantum((unsigned char) ReadBlobByte(image));
            else
              {
                blue=ScaleCharToQuantum((unsigned char) ReadBlobByte(image));
                green=ScaleCharToQuantum((unsigned char) ReadBlobByte(image));
                red=ScaleCharToQuantum((unsigned char) ReadBlobByte(image));
              }
          }
        if (image->storage_class == PseudoClass)
          SetPixelIndex(image,index,q);
        SetPixelBlue(image,blue,q);
        SetPixelGreen(image,green,q);
        SetPixelRed(image,red,q);
        length--;
        q+=GetPixelChannels(image);
      }
      if (x < (ssize_t) image->columns)
        break;
      if (SyncAuthenticPixels(image,exception) == MagickFalse)
        break;
      if (image->previous == (Image *) NULL)
        {
          status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
            image->rows);
          if (status == MagickFalse)
            break;
        }
    }
    if (image->storage_class == PseudoClass)
      (void) SyncImage(image,exception);
    if (EOFBlob(image) != MagickFalse)
      {
        ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
          image->filename);
        break;
      }
    /*
      Proceed to next image.
    */
    if (image_info->number_scenes != 0)
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    columns=ReadBlobMSBLong(image);
    height=ReadBlobMSBLong(image);
    (void) ReadBlobMSBShort(image);
    (void) ReadBlobMSBShort(image);
    bits_per_pixel=ReadBlobMSBShort(image);
    status=(columns != 0UL) && (height == 0UL) && ((bits_per_pixel == 8) ||
      (bits_per_pixel == 24)) ? MagickTrue : MagickFalse;
    if (status != MagickFalse)
      {
        /*
          Allocate next image structure.
        */
        AcquireNextImage(image_info,image,exception);
        if (GetNextImageInList(image) == (Image *) NULL)
          {
            image=DestroyImageList(image);
            return((Image *) NULL);
          }
        image=SyncNextImageInList(image);
        status=SetImageProgress(image,LoadImagesTag,TellBlob(image),
          GetBlobSize(image));
        if (status == MagickFalse)
          break;
      }
  } while (status != MagickFalse);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}