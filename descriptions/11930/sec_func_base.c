static Image *ReadCAPTIONImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    *caption,
    geometry[MagickPathExtent],
    *text;

  const char
    *gravity,
    *option;

  DrawInfo
    *draw_info;

  Image
    *image;

  MagickBooleanType
    split,
    status;

  register ssize_t
    i;

  size_t
    height,
    width;

  TypeMetric
    metrics;

  /*
    Initialize Image structure.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImage(image_info,exception);
  (void) ResetImagePixels(image,exception);
  (void) ResetImagePage(image,"0x0+0+0");
  /*
    Format caption.
  */
  option=GetImageOption(image_info,"filename");
  if (option == (const char *) NULL)
    caption=InterpretImageProperties((ImageInfo *) image_info,image,
      image_info->filename,exception);
  else
    if (LocaleNCompare(option,"caption:",8) == 0)
      caption=InterpretImageProperties((ImageInfo *) image_info,image,option+8,
        exception);
    else
      caption=InterpretImageProperties((ImageInfo *) image_info,image,option,
        exception);
  if (caption == (char *) NULL)
    return(DestroyImageList(image));
  (void) SetImageProperty(image,"caption",caption,exception);
  draw_info=CloneDrawInfo(image_info,(DrawInfo *) NULL);
  width=(size_t) floor(draw_info->pointsize*strlen(caption)+0.5);
  if (AcquireMagickResource(WidthResource,width) == MagickFalse)
    {
      caption=DestroyString(caption);
      draw_info=DestroyDrawInfo(draw_info);
      ThrowReaderException(ImageError,"WidthOrHeightExceedsLimit");
    }
  (void) CloneString(&draw_info->text,caption);
  gravity=GetImageOption(image_info,"gravity");
  if (gravity != (char *) NULL)
    draw_info->gravity=(GravityType) ParseCommandOption(MagickGravityOptions,
      MagickFalse,gravity);
  split=MagickFalse;
  status=MagickTrue;
  if (image->columns == 0)
    {
      text=AcquireString(caption);
      i=FormatMagickCaption(image,draw_info,split,&metrics,&text,
        exception);
      (void) CloneString(&draw_info->text,text);
      text=DestroyString(text);
      (void) FormatLocaleString(geometry,MagickPathExtent,"%+g%+g",
        -metrics.bounds.x1,metrics.ascent);
      if (draw_info->gravity == UndefinedGravity)
        (void) CloneString(&draw_info->geometry,geometry);
      status=GetMultilineTypeMetrics(image,draw_info,&metrics,exception);
      width=(size_t) floor(metrics.width+draw_info->stroke_width+0.5);
      image->columns=width;
    }
  if (image->rows == 0)
    {
      split=MagickTrue;
      text=AcquireString(caption);
      i=FormatMagickCaption(image,draw_info,split,&metrics,&text,exception);
      (void) CloneString(&draw_info->text,text);
      text=DestroyString(text);
      (void) FormatLocaleString(geometry,MagickPathExtent,"%+g%+g",
        -metrics.bounds.x1,metrics.ascent);
      if (draw_info->gravity == UndefinedGravity)
        (void) CloneString(&draw_info->geometry,geometry);
      status=GetMultilineTypeMetrics(image,draw_info,&metrics,exception);
      image->rows=(size_t) ((i+1)*(metrics.ascent-metrics.descent+
        draw_info->interline_spacing+draw_info->stroke_width)+0.5);
    }
  if (status != MagickFalse)
    status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    { 
      caption=DestroyString(caption);
      draw_info=DestroyDrawInfo(draw_info);
      return(DestroyImageList(image));
    }
  if (SetImageBackgroundColor(image,exception) == MagickFalse)
    {
      caption=DestroyString(caption);
      draw_info=DestroyDrawInfo(draw_info);
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  if ((fabs(image_info->pointsize) < MagickEpsilon) && (strlen(caption) > 0))
    {
      double
        high,
        low;

      ssize_t
        n;

      /*
        Auto fit text into bounding box.
      */
      for (n=0; n < 32; n++, draw_info->pointsize*=2.0)
      {
        text=AcquireString(caption);
        i=FormatMagickCaption(image,draw_info,split,&metrics,&text,
          exception);
        (void) CloneString(&draw_info->text,text);
        text=DestroyString(text);
        (void) FormatLocaleString(geometry,MagickPathExtent,"%+g%+g",
          -metrics.bounds.x1,metrics.ascent);
        if (draw_info->gravity == UndefinedGravity)
          (void) CloneString(&draw_info->geometry,geometry);
        status=GetMultilineTypeMetrics(image,draw_info,&metrics,exception);
        if (status == MagickFalse)
          break;
        width=(size_t) floor(metrics.width+draw_info->stroke_width+0.5);
        height=(size_t) floor(metrics.height+draw_info->stroke_width+0.5);
        if ((image->columns != 0) && (image->rows != 0))
          {
            if ((width >= image->columns) && (height >= image->rows))
              break;
          }
        else
          if (((image->columns != 0) && (width >= image->columns)) ||
              ((image->rows != 0) && (height >= image->rows)))
            break;
      }
      high=draw_info->pointsize;
      for (low=1.0; (high-low) > 0.5; )
      {
        draw_info->pointsize=(low+high)/2.0;
        text=AcquireString(caption);
        i=FormatMagickCaption(image,draw_info,split,&metrics,&text,
          exception);
        (void) CloneString(&draw_info->text,text);
        text=DestroyString(text);
        (void) FormatLocaleString(geometry,MagickPathExtent,"%+g%+g",
          -metrics.bounds.x1,metrics.ascent);
        if (draw_info->gravity == UndefinedGravity)
          (void) CloneString(&draw_info->geometry,geometry);
        status=GetMultilineTypeMetrics(image,draw_info,&metrics,exception);
        if (status == MagickFalse)
          break;
        width=(size_t) floor(metrics.width+draw_info->stroke_width+0.5);
        height=(size_t) floor(metrics.height+draw_info->stroke_width+0.5);
        if ((image->columns != 0) && (image->rows != 0))
          {
            if ((width < image->columns) && (height < image->rows))
              low=draw_info->pointsize+0.5;
            else
              high=draw_info->pointsize-0.5;
          }
        else
          if (((image->columns != 0) && (width < image->columns)) ||
              ((image->rows != 0) && (height < image->rows)))
            low=draw_info->pointsize+0.5;
          else
            high=draw_info->pointsize-0.5;
      }
      draw_info->pointsize=floor((low+high)/2.0-0.5);
    }
  /*
    Draw caption.
  */
  i=FormatMagickCaption(image,draw_info,split,&metrics,&caption,exception);
  (void) CloneString(&draw_info->text,caption);
  caption=DestroyString(caption);
  (void) FormatLocaleString(geometry,MagickPathExtent,"%+g%+g",MagickMax(
    draw_info->direction == RightToLeftDirection ? (double) image->columns-
    metrics.bounds.x2 : -metrics.bounds.x1,0.0),draw_info->gravity ==
    UndefinedGravity ? MagickMax(metrics.ascent,metrics.bounds.y2) : 0.0);
  (void) CloneString(&draw_info->geometry,geometry);
  status=AnnotateImage(image,draw_info,exception);
  if (image_info->pointsize == 0.0)
    { 
      char
        pointsize[MagickPathExtent];
      
      (void) FormatLocaleString(pointsize,MagickPathExtent,"%.20g",
        draw_info->pointsize);
      (void) SetImageProperty(image,"caption:pointsize",pointsize,exception);
    }
  draw_info=DestroyDrawInfo(draw_info);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  return(GetFirstImageInList(image));
}