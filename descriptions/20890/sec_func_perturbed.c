static Image *ReadGRADIENTImage(const ImageInfo *image_info,
  ExceptionInfo *errorinfo)
{
  char
    start_color[MagickPathExtent],
    stop_color[MagickPathExtent];

  Image
    *image;

  ImageInfo
    *read_info;

  MagickBooleanType
    status;

  StopInfo
    *stops;

  /*
    Identify start and stop gradient colors.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(errorinfo != (ExceptionInfo *) NULL);
  assert(errorinfo->signature == MagickCoreSignature);
  (void) CopyMagickString(start_color,"white",MagickPathExtent);
  (void) CopyMagickString(stop_color,"black",MagickPathExtent);
  if (*image_info->filename != '\0')
    {
      register char
        *p;

      (void) CopyMagickString(start_color,image_info->filename,
        MagickPathExtent);
      for (p=start_color; (*p != '-') && (*p != '\0'); p++)
        if (*p == '(')
          {
            for (p++; (*p != ')') && (*p != '\0'); p++);
            if (*p == '\0')
              break;
          }
      if (*p == '-')
        (void) CopyMagickString(stop_color,p+1,MagickPathExtent);
      *p='\0';
    }
  /*
    Create base gradient image from start color.
  */
  read_info=CloneImageInfo(image_info);
  SetImageInfoBlob(read_info,(void *) NULL,0);
  (void) CopyMagickString(read_info->filename,start_color,MagickPathExtent);
  image=ReadXCImage(read_info,errorinfo);
  read_info=DestroyImageInfo(read_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Create gradient stops.
  */
  stops=(StopInfo *) AcquireQuantumMemory(2,sizeof(*stops));
  if (stops == (StopInfo *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  stops[0].offset=0.0;
  stops[1].offset=1.0;
  status=QueryColorCompliance(start_color,AllCompliance,&stops[0].color,
    errorinfo);
  if (status != MagickFalse)
    status=QueryColorCompliance(stop_color,AllCompliance,&stops[1].color,
      errorinfo);
  if (status == MagickFalse)
    {
      stops=(StopInfo *) RelinquishMagickMemory(stops);
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  (void) SetImageColorspace(image,stops[0].color.colorspace,errorinfo);
  if ((stops[0].color.alpha_trait != UndefinedPixelTrait) ||
      (stops[1].color.alpha_trait != UndefinedPixelTrait))
    SetImageAlpha(image,TransparentAlpha,errorinfo);
  /*
    Paint gradient.
  */
  status=GradientImage(image,LocaleCompare(image_info->magick,"GRADIENT") == 0 ?
    LinearGradient : RadialGradient,PadSpread,stops,2,errorinfo);
  stops=(StopInfo *) RelinquishMagickMemory(stops);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  return(GetFirstImageInList(image));
}