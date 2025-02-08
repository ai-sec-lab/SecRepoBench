if ((stops[0].color.alpha_trait != UndefinedPixelTrait) ||
      (stops[1].color.alpha_trait != UndefinedPixelTrait))
    SetImageAlpha(image,TransparentAlpha,exception);
  /*
    Paint gradient.
  */
  status=GradientImage(image,LocaleCompare(image_info->magick,"GRADIENT") == 0 ?
    LinearGradient : RadialGradient,PadSpread,stops,2,exception);
  stops=(StopInfo *) RelinquishMagickMemory(stops);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  return(GetFirstImageInList(image));