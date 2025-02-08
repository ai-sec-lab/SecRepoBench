#define ThrowBAYERException \
{ \
  if (bayer_b != (Image *) NULL) \
    bayer_b=DestroyImage(bayer_b); \
  if (bayer_g0 != (Image *) NULL) \
    bayer_g0=DestroyImage(bayer_g0); \
  if (bayer_g1 != (Image *) NULL) \
    bayer_g1=DestroyImage(bayer_g1); \
  if (bayer_image != (Image *) NULL) \
    bayer_image=DestroyImage(bayer_image); \
  if (images != (Image *) NULL) \
    images=DestroyImageList(images); \
  return(MagickFalse); \
}
  Image
    *bayer_b=(Image *) NULL,
    *bayer_g0=(Image *) NULL,
    *bayer_g1=(Image *) NULL,
    *bayer_image=(Image *) NULL,
    *images=(Image *) NULL;

  MagickBooleanType
    status;

  PixelInfo
    pixel;

  Quantum
    *q;

  /*
    Deconstruct RGB image into a single channel RGGB raw image.
  */
  assert(imginfo != (const ImageInfo *) NULL);
  assert(imginfo->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  bayer_g0=AcquireImage((ImageInfo *) NULL,exception);
  if (bayer_g0 == (Image *) NULL)
    return(MagickFalse);
  (void) SetImageExtent(bayer_g0,2,2,exception);
  (void) QueryColorCompliance("#000",AllCompliance,
    &bayer_g0->background_color,exception);
  (void) SetImageBackgroundColor(bayer_g0,exception);
  q=GetAuthenticPixels(bayer_g0,1,0,1,1,exception);
  if (q == (Quantum *) NULL)
    ThrowBAYERException;
  (void) QueryColorCompliance("#fff",AllCompliance,&pixel,exception);
  SetPixelViaPixelInfo(bayer_g0,&pixel,q);
  bayer_b=RollImage(bayer_g0,0,1,exception);
  if (bayer_b == (Image *) NULL)
    ThrowBAYERException;
  bayer_g1=RollImage(bayer_b,-1,0,exception);
  if (bayer_g1 == (Image *) NULL)
    ThrowBAYERException;
  bayer_image=CloneImage(image,0,0,MagickTrue,exception);
  status=MagickFalse;
  if (bayer_image == (Image *) NULL)
    ThrowBAYERException;
  (void) SetImageColorspace(bayer_image,sRGBColorspace,exception);
  (void) SetPixelChannelMask(bayer_image,RedChannel|GreenChannel|BlueChannel);
  images=SeparateImages(bayer_image,exception);
  if (images == (Image *) NULL)
    ThrowBAYERException;
  bayer_image=DestroyImage(bayer_image);
  bayer_image=BayerApplyMask(images,0,1,bayer_g0,exception);
  if (bayer_image == (Image *) NULL)
    ThrowBAYERException;
  AppendImageToList(&images,bayer_image);
  bayer_image=BayerApplyMask(images,3,1,bayer_g1,exception);
  if (bayer_image == (Image *) NULL)
    ThrowBAYERException;
  AppendImageToList(&images,bayer_image);
  bayer_image=BayerApplyMask(images,4,2,bayer_b,exception);
  if (bayer_image != (Image *) NULL)
    status=MagickTrue;
  images=DestroyImageList(images);
  bayer_g1=DestroyImage(bayer_g1);
  bayer_g0=DestroyImage(bayer_g0);
  bayer_b=DestroyImage(bayer_b);
  if (bayer_image != (Image *) NULL)
    {
      ImageInfo
        *write_info;

      write_info=CloneImageInfo(imginfo);
      write_info->verbose=MagickFalse;
      (void) CopyMagickString(write_info->magick,"GRAY",MagickPathExtent);
      (void) CopyMagickString(bayer_image->filename,image->filename,
        MagickPathExtent);
      status=WriteImage(write_info,bayer_image,exception);
      bayer_image=DestroyImage(bayer_image);
      write_info=DestroyImageInfo(write_info);
    }