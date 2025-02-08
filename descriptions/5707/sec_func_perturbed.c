static Image *ReadWBMPImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  Image
    *image;

  int
    bytevalue;

  MagickBooleanType
    status;

  register ssize_t
    x;

  register Quantum
    *q;

  ssize_t
    y;

  unsigned char
    bit;

  unsigned short
    header;

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
  header=0;
  if (ReadBlob(image,2,(unsigned char *) &header) == 0)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  if (header != 0)
    ThrowReaderException(CoderError,"OnlyLevelZerofilesSupported");
  /*
    Initialize image structure.
  */
  if (WBMPReadInteger(image,&image->columns) == MagickFalse)
    ThrowReaderException(CorruptImageError,"CorruptWBMPimage");
  if (WBMPReadInteger(image,&image->rows) == MagickFalse)
    ThrowReaderException(CorruptImageError,"CorruptWBMPimage");
  if ((image->columns == 0) || (image->rows == 0))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  if (DiscardBlobBytes(image,image->offset) == MagickFalse)
    ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  if (image_info->ping != MagickFalse)
    {
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  (void) SetImageBackgroundColor(image,exception);
  if (AcquireImageColormap(image,2,exception) == MagickFalse)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  /*
    Convert bi-level image to pixel packets.
  */
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    bit=0;
    bytevalue=0;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (bit == 0)
        {
          bytevalue=ReadBlobByte(image);
          if (bytevalue == EOF)
            ThrowReaderException(CorruptImageError,"CorruptImage");
        }
      SetPixelIndex(image,(bytevalue & (0x01 << (7-bit))) ? 1 : 0,q);
      bit++;
      if (bit == 8)
        bit=0;
      q+=GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
    status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
                image->rows);
    if (status == MagickFalse)
      break;
  }
  (void) SyncImage(image,exception);
  if (EOFBlob(image) != MagickFalse)
    ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}