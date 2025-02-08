Image
    *bayer_image=(Image *) NULL,
    *images=(Image *) NULL;

  MagickBooleanType
    status;

  /*
    Deconstruct RGB image into a single channel RGGB raw image.
  */
  assert(imginfo != (const ImageInfo *) NULL);
  assert(imginfo->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  bayer_image=CloneImage(image,0,0,MagickTrue,exception);
  status=MagickFalse;
  if (bayer_image == (Image *) NULL)
    return(MagickFalse);
  (void) SetImageColorspace(bayer_image,sRGBColorspace,exception);
  (void) SetPixelChannelMask(bayer_image,RedChannel|GreenChannel|BlueChannel);
  images=SeparateImages(bayer_image,exception);
  bayer_image=DestroyImage(bayer_image);
  if (images == (Image *) NULL)
    return(MagickFalse);
  bayer_image=BayerApplyMask(images,0,1,1,0,exception);
  if (bayer_image == (Image *) NULL)
    {
      images=DestroyImageList(images);
      return(MagickFalse);
    }
  AppendImageToList(&images,bayer_image);
  bayer_image=BayerApplyMask(images,3,1,0,1,exception);
  if (bayer_image == (Image *) NULL)
    {
      images=DestroyImageList(images);
      return(MagickFalse);
    }
  AppendImageToList(&images,bayer_image);
  status=MagickFalse;
  bayer_image=BayerApplyMask(images,4,2,1,1,exception);
  if (bayer_image != (Image *) NULL)
    status=MagickTrue;
  images=DestroyImageList(images);
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