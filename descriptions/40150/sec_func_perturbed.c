static Image *ReadLABELImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    geometry[MagickPathExtent],
    *label;

  DrawInfo
    *draw_info;

  Image
    *image;

  MagickBooleanType
    status;

  TypeMetric
    metrics;

  size_t
    imgheight,
    width;

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
  (void) ResetImagePage(image,"0x0+0+0");
  if ((image->columns != 0) && (image->rows != 0))
    {
      status=SetImageExtent(image,image->columns,image->rows,exception);
      if (status == MagickFalse)
        return(DestroyImageList(image));
      (void) SetImageBackgroundColor(image,exception);
    }
  label=InterpretImageProperties((ImageInfo *) image_info,image,
    image_info->filename,exception);
  if (label == (char *) NULL)
    return(DestroyImageList(image));
  (void) SetImageProperty(image,"label",label,exception);
  draw_info=CloneDrawInfo(image_info,(DrawInfo *) NULL);
  width=(size_t) floor(0.5*draw_info->pointsize*strlen(label)+0.5);
  if (AcquireMagickResource(WidthResource,width) == MagickFalse)
    {
      label=DestroyString(label);
      draw_info=DestroyDrawInfo(draw_info);
      ThrowReaderException(ImageError,"WidthOrHeightExceedsLimit");
    }
  draw_info->text=ConstantString(label);
  (void) memset(&metrics,0,sizeof(metrics));
  status=GetMultilineTypeMetrics(image,draw_info,&metrics,exception);
  AdjustTypeMetricBounds(&metrics);
  if ((image->columns == 0) && (image->rows == 0))
    {
      image->columns=(size_t) floor(metrics.width+draw_info->stroke_width+0.5);
      image->rows=(size_t) floor(metrics.height+draw_info->stroke_width+0.5);
    }
  else
    if ((status != MagickFalse) && (strlen(label) > 0) &&
        (((image->columns == 0) || (image->rows == 0)) ||
         (fabs(image_info->pointsize) < MagickEpsilon)))
      {
        const char
          *option;

        double
          high,
          low;

        ssize_t
          n;

        /*
          Auto fit text into bounding box.
        */
        low=1.0;
        option=GetImageOption(image_info,"label:max-pointsize");
        if (option != (const char*) NULL)
          {
            high=StringToDouble(option,(char**) NULL);
            if (high < 1.0)
              high=1.0;
            high+=1.0;
          }
        else
          {
            option=GetImageOption(image_info,"label:start-pointsize");
            if (option != (const char *) NULL)
              {
                draw_info->pointsize=StringToDouble(option,(char**) NULL);
                if (draw_info->pointsize < 1.0)
                  draw_info->pointsize=1.0;
              }
            for (n=0; n < 32; n++, draw_info->pointsize*=2.0)
            {
              (void) FormatLocaleString(geometry,MagickPathExtent,"%+g%+g",
                metrics.bounds.x1,metrics.ascent);
              if (draw_info->gravity == UndefinedGravity)
                (void) CloneString(&draw_info->geometry,geometry);
              status=GetMultilineTypeMetrics(image,draw_info,&metrics,exception);
              if (status == MagickFalse)
                break;
              AdjustTypeMetricBounds(&metrics);
              width=(size_t) floor(metrics.width+draw_info->stroke_width+0.5);
              imgheight=(size_t) floor(metrics.height+draw_info->stroke_width+0.5);
              if ((image->columns != 0) && (image->rows != 0))
                {
                  if ((width >= image->columns) || (imgheight >= image->rows))
                    break;
                  if ((width < image->columns) && (imgheight < image->rows))
                    low=draw_info->pointsize;
                }
              else
                if (((image->columns != 0) && (width >= image->columns)) ||
                    ((image->rows != 0) && (imgheight >= image->rows)))
                  break;
            }
            if (status == MagickFalse)
              {
                label=DestroyString(label);
                draw_info=DestroyDrawInfo(draw_info);
                image=DestroyImageList(image);
                return((Image *) NULL);
              }
          }
        high=draw_info->pointsize;
        while((high-low) > 0.5)
        {
          draw_info->pointsize=(low+high)/2.0;
          (void) FormatLocaleString(geometry,MagickPathExtent,"%+g%+g",
            metrics.bounds.x1,metrics.ascent);
          if (draw_info->gravity == UndefinedGravity)
            (void) CloneString(&draw_info->geometry,geometry);
          status=GetMultilineTypeMetrics(image,draw_info,&metrics,exception);
          if (status == MagickFalse)
            break;
          AdjustTypeMetricBounds(&metrics);
          width=(size_t) floor(metrics.width+draw_info->stroke_width+0.5);
          imgheight=(size_t) floor(metrics.height+draw_info->stroke_width+0.5);
          if ((image->columns != 0) && (image->rows != 0))
            {
              if ((width < image->columns) && (imgheight < image->rows))
                low=draw_info->pointsize+0.5;
              else
                high=draw_info->pointsize-0.5;
            }
          else
            if (((image->columns != 0) && (width < image->columns)) ||
                ((image->rows != 0) && (imgheight < image->rows)))
              low=draw_info->pointsize+0.5;
            else
              high=draw_info->pointsize-0.5;
        }
        if (status != MagickFalse)
          {
            draw_info->pointsize=floor((low+high)/2.0-0.5);
            status=GetMultilineTypeMetrics(image,draw_info,&metrics,exception);
            AdjustTypeMetricBounds(&metrics);
          }
      }
   label=DestroyString(label);
   if (status == MagickFalse)
     {
       draw_info=DestroyDrawInfo(draw_info);
       image=DestroyImageList(image);
       return((Image *) NULL);
     }
  if (image->columns == 0)
    image->columns=(size_t) floor(metrics.width+draw_info->stroke_width+0.5);
  if (image->columns == 0)
    image->columns=(size_t) floor(draw_info->pointsize+draw_info->stroke_width+
      0.5);
  if (image->rows == 0)
    image->rows=(size_t) floor(metrics.height+draw_info->stroke_width+0.5);
  if (image->rows == 0)
    image->rows=(size_t) floor(draw_info->pointsize+draw_info->stroke_width+
      0.5);
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    {
      draw_info=DestroyDrawInfo(draw_info);
      return(DestroyImageList(image));
    }
  if (SetImageBackgroundColor(image,exception) == MagickFalse)
    {
      draw_info=DestroyDrawInfo(draw_info);
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Draw label.
  */
  (void) FormatLocaleString(geometry,MagickPathExtent,"%+g%+g",
    (draw_info->direction == RightToLeftDirection ? (double) image->columns-
    metrics.bounds.x2 : metrics.bounds.x1),(draw_info->gravity ==
    UndefinedGravity ? MagickMax(metrics.ascent,metrics.bounds.y2) : 0.0));
  (void) CloneString(&draw_info->geometry,geometry);
  status=AnnotateImage(image,draw_info,exception);
  if (image_info->pointsize == 0.0)
    (void) FormatImageProperty(image,"label:pointsize","%.20g",
      draw_info->pointsize);
  draw_info=DestroyDrawInfo(draw_info);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  return(GetFirstImageInList(image));
}