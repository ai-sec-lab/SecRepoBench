static Image *ReadQOIImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image;

  Quantum
    *q;

  MagickBooleanType
    status;

  size_t
     colorspace,
     channels;

  ssize_t
    count,
    i;

  unsigned char
    magick[4];

  qoi_rgba_t
    px,
    lut[64];

  int
    b,
    vg,
    run;

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
    Initialize image structure.
  */
  image->endian=MSBEndian;
  image->depth=8;
  count=ReadBlob(image,4,magick);
  if ((count != 4) || (LocaleNCompare((char *) magick, "qoif", 4) != 0))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  image->columns=(size_t) ReadBlobMSBLong(image);
  image->rows=(size_t) ReadBlobMSBLong(image);
  if (image->columns == 0 || image->rows == 0)
    ThrowReaderException(CorruptImageError,"NegativeOrZeroImageSize");
  if (image->ping != MagickFalse) {
    (void) CloseBlob(image);
    return(GetFirstImageInList(image));
  }

  channels=(size_t) ReadBlobByte(image);
  if (channels == 3)
    SetQuantumImageType(image,RGBQuantum);
  else if (channels == 4)
    {
      SetQuantumImageType(image,RGBAQuantum);
      image->alpha_trait=BlendPixelTrait;
    }
  else
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");

  colorspace=(size_t) ReadBlobByte(image);
  if (colorspace == QOI_SRGB)
    (void) SetImageColorspace(image,sRGBColorspace,exception);
  else if (colorspace == QOI_LINEAR)
    (void) SetImageColorspace(image,RGBColorspace,exception);
  else
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");

  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));

  /*
    Get a write pointer for the whole image.
  */
  q=QueueAuthenticPixels(image,0,0,image->columns,image->rows,exception);
  if (q == (Quantum *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  /*
    Initialize decoding state.
  */
  px.rgba.r=0;
  px.rgba.g=0;
  px.rgba.b=0;
  px.rgba.a=255;
  memset(lut,0,sizeof(lut));
  /*
    Actual decoding.
  */
  for (i=0; i < (ssize_t) (image->rows * image->columns);)
  {
    run = 0;

    if ((b=ReadBlobByte(image)) == EOF)
      break;
    if (b == QOI_OP_RGB) {
      if ((b=ReadBlobByte(image)) == EOF)
        break;
      px.rgba.r=(unsigned char) b;
      if ((b=ReadBlobByte(image)) == EOF)
        break;
      px.rgba.g=(unsigned char) b;
      if ((b=ReadBlobByte(image)) == EOF)
        break;
      px.rgba.b=(unsigned char) b;
    }
    else if (b == QOI_OP_RGBA) {
      if ((b=ReadBlobByte(image)) == EOF)
        break;
      px.rgba.r=(unsigned char) b;
      if ((b=ReadBlobByte(image)) == EOF)
        break;
      px.rgba.g=(unsigned char) b;
      if ((b=ReadBlobByte(image)) == EOF)
        break;
      px.rgba.b=(unsigned char) b;
      if ((b=ReadBlobByte(image)) == EOF)
        break;
      px.rgba.a=(unsigned char) b;
    }
    else if ((b & QOI_MASK_2) == QOI_OP_INDEX) {
      px=lut[b & ~QOI_MASK_2];
    }
    else if ((b & QOI_MASK_2) == QOI_OP_DIFF) {
      px.rgba.r+=((b >> 4) & 0x03) - 2;
      px.rgba.g+=((b >> 2) & 0x03) - 2;
      px.rgba.b+=( b       & 0x03) - 2;
    }
    else if ((b & QOI_MASK_2) == QOI_OP_LUMA) {
      vg=(b & ~QOI_MASK_2) - 32;
      if ((b=ReadBlobByte(image)) == EOF)
        break;
      px.rgba.r+=vg - 8 + ((b >> 4) & 0x0f);
      px.rgba.g+=vg;
      px.rgba.b+=vg - 8 +  (b       & 0x0f);
    }
    else if ((b & QOI_MASK_2) == QOI_OP_RUN) {
      run=b & ~QOI_MASK_2;
    }
    lut[QOI_COLOR_HASH(px) % 64]=px;
    do {
      if (i < (ssize_t) (image->columns*image->rows))
        {
          SetPixelRed(image,ScaleCharToQuantum((unsigned char)px.rgba.r),q);
          SetPixelGreen(image,ScaleCharToQuantum((unsigned char)px.rgba.g),q);
          SetPixelBlue(image,ScaleCharToQuantum((unsigned char)px.rgba.b),q);
          if (channels == 4)
            SetPixelAlpha(image,ScaleCharToQuantum((unsigned char) px.rgba.a),q);
        }
      q+=GetPixelChannels(image);
      i++;
    } while (run-- > 0);
    status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) i,image->rows * image->columns);
    if (status == MagickFalse)
      break;
  }
  status=SyncAuthenticPixels(image,exception);
  if (i < (ssize_t) (image->columns*image->rows))
    ThrowReaderException(CorruptImageError,"NotEnoughPixelData");
  if (EOFBlob(image) != MagickFalse)
    ThrowFileException(exception,CorruptImageError,
      "UnexpectedEndOfFile",image->filename);
  (void) CloseBlob(image);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  return(GetFirstImageInList(image));
}