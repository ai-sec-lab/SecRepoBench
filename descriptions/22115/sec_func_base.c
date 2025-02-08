static Image *ReadTXTImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    colorspace[MagickPathExtent],
    text[MagickPathExtent];

  double
    max_value,
    x_offset,
    y_offset;

  Image
    *image;

  PixelInfo
    pixel;

  MagickBooleanType
    status;

  QuantumAny
    range;

  register ssize_t
    i,
    x;

  register Quantum
    *q;

  ssize_t
    count,
    type,
    y;

  unsigned long
    depth,
    height,
    width;

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
  (void) memset(text,0,sizeof(text));
  (void) ReadBlobString(image,text);
  if (LocaleNCompare((char *) text,MagickTXTID,strlen(MagickTXTID)) != 0)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  x_offset=(-1.0);
  y_offset=(-1.0);
  q=(Quantum *) NULL;
  do
  {
    width=0;
    height=0;
    max_value=0.0;
    *colorspace='\0';
    count=(ssize_t) sscanf(text+32,"%lu,%lu,%lf,%32s",&width,&height,&max_value,
      colorspace);
    if ((count != 4) || (width == 0) || (height == 0) || (max_value == 0.0))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    image->columns=width;
    image->rows=height;
    if ((max_value == 0.0) || (max_value > 18446744073709551615.0))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    for (depth=1; (GetQuantumRange(depth)+1.0) < max_value; depth++) ;
    image->depth=depth;
    status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status != MagickFalse)
      status=ResetImagePixels(image,exception);
    if (status == MagickFalse)
      return(DestroyImageList(image));
    LocaleLower(colorspace);
    i=(ssize_t) strlen(colorspace)-1;
    image->alpha_trait=UndefinedPixelTrait;
    if ((i > 0) && (colorspace[i] == 'a'))
      {
        colorspace[i]='\0';
        image->alpha_trait=BlendPixelTrait;
      }
    type=ParseCommandOption(MagickColorspaceOptions,MagickFalse,colorspace);
    if (type < 0)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    (void) SetImageColorspace(image,(ColorspaceType) type,exception);
    (void) SetImageBackgroundColor(image,exception);
    GetPixelInfo(image,&pixel);
    range=GetQuantumRange(image->depth);
    status=MagickTrue;
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      double
        alpha,
        black,
        blue,
        green,
        red;

      if (status == MagickFalse)
        break;
      red=0.0;
      green=0.0;
      blue=0.0;
      black=0.0;
      alpha=0.0;
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        if (ReadBlobString(image,text) == (char *) NULL)
          {
            status=MagickFalse;
            break;
          }
        switch (image->colorspace)
        {
          case LinearGRAYColorspace:
          case GRAYColorspace:
          {
            if (image->alpha_trait != UndefinedPixelTrait)
              {
                count=(ssize_t) sscanf(text,"%lf,%lf: (%lf%*[%,]%lf%*[%,]",
                  &x_offset,&y_offset,&red,&alpha);
                green=red;
                blue=red;
                break;
              }
            count=(ssize_t) sscanf(text,"%lf,%lf: (%lf%*[%,]",&x_offset,
              &y_offset,&red);
            green=red;
            blue=red;
            break;
          }
          case CMYKColorspace:
          {
            if (image->alpha_trait != UndefinedPixelTrait)
              {
                count=(ssize_t) sscanf(text,
                  "%lf,%lf: (%lf%*[%,]%lf%*[%,]%lf%*[%,]%lf%*[%,]%lf%*[%,]",
                  &x_offset,&y_offset,&red,&green,&blue,&black,&alpha);
                break;
              }
            count=(ssize_t) sscanf(text,
              "%lf,%lf: (%lf%*[%,]%lf%*[%,]%lf%*[%,]%lf%*[%,]",&x_offset,
              &y_offset,&red,&green,&blue,&black);
            break;
          }
          default:
          {
            if (image->alpha_trait != UndefinedPixelTrait)
              {
                count=(ssize_t) sscanf(text,
                  "%lf,%lf: (%lf%*[%,]%lf%*[%,]%lf%*[%,]%lf%*[%,]",
                  &x_offset,&y_offset,&red,&green,&blue,&alpha);
                break;
              }
            count=(ssize_t) sscanf(text,"%lf,%lf: (%lf%*[%,]%lf%*[%,]%lf%*[%,]",
              &x_offset,&y_offset,&red,&green,&blue);
            break;
          }
        }
        if (strchr(text,'%') != (char *) NULL)
          {
            red*=0.01*range;
            green*=0.01*range;
            blue*=0.01*range;
            black*=0.01*range;
            alpha*=0.01*range;
          }
        if (image->colorspace == LabColorspace)
          {
            green+=(range+1)/2.0;
            blue+=(range+1)/2.0;
          }
        pixel.red=(MagickRealType) ScaleAnyToQuantum((QuantumAny)
          MagickMax(red+0.5,0.0),range);
        pixel.green=(MagickRealType) ScaleAnyToQuantum((QuantumAny)
          MagickMax(green+0.5,0.0),range);
        pixel.blue=(MagickRealType) ScaleAnyToQuantum((QuantumAny)
          MagickMax(blue+0.5,0.0),range);
        pixel.black=(MagickRealType) ScaleAnyToQuantum((QuantumAny)
          MagickMax(black+0.5,0.0),range);
        pixel.alpha=(MagickRealType) ScaleAnyToQuantum((QuantumAny)
          MagickMax(alpha+0.5,0.0),range);
        q=GetAuthenticPixels(image,(ssize_t) x_offset,(ssize_t) y_offset,1,1,
          exception);
        if (q == (Quantum *) NULL)
          {
            status=MagickFalse;
            break;
          }
        SetPixelViaPixelInfo(image,&pixel,q);
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          {
            status=MagickFalse;
            break;
          }
      }
    }
    if (status == MagickFalse)
      break;
    *text='\0';
    (void) ReadBlobString(image,text);
    if (LocaleNCompare((char *) text,MagickTXTID,strlen(MagickTXTID)) == 0)
      {
        /*
          Allocate next image structure.
        */
        AcquireNextImage(image_info,image,exception);
        if (GetNextImageInList(image) == (Image *) NULL)
          {
            status=MagickFalse;
            break;
          }
        image=SyncNextImageInList(image);
        status=SetImageProgress(image,LoadImagesTag,TellBlob(image),
          GetBlobSize(image));
        if (status == MagickFalse)
          break;
      }
  } while (LocaleNCompare((char *) text,MagickTXTID,strlen(MagickTXTID)) == 0);
  (void) CloseBlob(image);
  if (q == (Quantum *) NULL)
    return(DestroyImage(image));
  return(GetFirstImageInList(image));
}