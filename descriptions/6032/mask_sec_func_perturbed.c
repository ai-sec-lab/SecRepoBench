static MagickBooleanType WriteHDRImage(const ImageInfo *image_info,Image *img,
  ExceptionInfo *exception)
{
  char
    header[MagickPathExtent];

  const char
    *property;

  MagickBooleanType
    status;

  register const Quantum
    *p;

  register ssize_t
    i,
    x;

  size_t
    length;

  ssize_t
    count,
    y;

  unsigned char
    pixel[4],
    *pixels;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(img != (Image *) NULL);
  assert(img->signature == MagickCoreSignature);
  if (img->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",img->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  status=OpenBlob(image_info,img,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  if (IsRGBColorspace(img->colorspace) == MagickFalse)
    (void) TransformImageColorspace(img,RGBColorspace,exception);
  /*
    Write header.
  */
  (void) ResetMagickMemory(header,' ',MagickPathExtent);
  length=CopyMagickString(header,"#?RADIANCE\n",MagickPathExtent);
  (void) WriteBlob(img,length,(unsigned char *) header);
  property=GetImageProperty(img,"comment",exception);
  if ((property != (const char *) NULL) &&
      (strchr(property,'\n') == (char *) NULL))
    {
      // <MASK>
    }
  property=GetImageProperty(img,"hdr:exposure",exception);
  if (property != (const char *) NULL)
    {
      count=FormatLocaleString(header,MagickPathExtent,"EXPOSURE=%g\n",
        strtod(property,(char **) NULL));
      (void) WriteBlob(img,(size_t) count,(unsigned char *) header);
    }
  if (img->gamma != 0.0)
    {
      count=FormatLocaleString(header,MagickPathExtent,"GAMMA=%g\n",
        img->gamma);
      (void) WriteBlob(img,(size_t) count,(unsigned char *) header);
    }
  count=FormatLocaleString(header,MagickPathExtent,
    "PRIMARIES=%g %g %g %g %g %g %g %g\n",
    img->chromaticity.red_primary.x,img->chromaticity.red_primary.y,
    img->chromaticity.green_primary.x,img->chromaticity.green_primary.y,
    img->chromaticity.blue_primary.x,img->chromaticity.blue_primary.y,
    img->chromaticity.white_point.x,img->chromaticity.white_point.y);
  (void) WriteBlob(img,(size_t) count,(unsigned char *) header);
  length=CopyMagickString(header,"FORMAT=32-bit_rle_rgbe\n\n",MagickPathExtent);
  (void) WriteBlob(img,length,(unsigned char *) header);
  count=FormatLocaleString(header,MagickPathExtent,"-Y %.20g +X %.20g\n",
    (double) img->rows,(double) img->columns);
  (void) WriteBlob(img,(size_t) count,(unsigned char *) header);
  /*
    Write HDR pixels.
  */
  pixels=(unsigned char *) AcquireQuantumMemory(img->columns+128,4*
    sizeof(*pixels));
  if (pixels == (unsigned char *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(pixels,0,4*(img->columns+128)*sizeof(*pixels));
  for (y=0; y < (ssize_t) img->rows; y++)
  {
    p=GetVirtualPixels(img,0,y,img->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    if ((img->columns >= 8) && (img->columns <= 0x7ffff))
      {
        pixel[0]=2;
        pixel[1]=2;
        pixel[2]=(unsigned char) (img->columns >> 8);
        pixel[3]=(unsigned char) (img->columns & 0xff);
        count=WriteBlob(img,4*sizeof(*pixel),pixel);
        if (count != (ssize_t) (4*sizeof(*pixel)))
          break;
      }
    i=0;
    for (x=0; x < (ssize_t) img->columns; x++)
    {
      double
        gamma;

      pixel[0]=0;
      pixel[1]=0;
      pixel[2]=0;
      pixel[3]=0;
      gamma=QuantumScale*GetPixelRed(img,p);
      if ((QuantumScale*GetPixelGreen(img,p)) > gamma)
        gamma=QuantumScale*GetPixelGreen(img,p);
      if ((QuantumScale*GetPixelBlue(img,p)) > gamma)
        gamma=QuantumScale*GetPixelBlue(img,p);
      if (gamma > MagickEpsilon)
        {
          int
            exponent;

          gamma=frexp(gamma,&exponent)*256.0/gamma;
          pixel[0]=(unsigned char) (gamma*QuantumScale*GetPixelRed(img,p));
          pixel[1]=(unsigned char) (gamma*QuantumScale*GetPixelGreen(img,p));
          pixel[2]=(unsigned char) (gamma*QuantumScale*GetPixelBlue(img,p));
          pixel[3]=(unsigned char) (exponent+128);
        }
      if ((img->columns >= 8) && (img->columns <= 0x7ffff))
        {
          pixels[x]=pixel[0];
          pixels[x+img->columns]=pixel[1];
          pixels[x+2*img->columns]=pixel[2];
          pixels[x+3*img->columns]=pixel[3];
        }
      else
        {
          pixels[i++]=pixel[0];
          pixels[i++]=pixel[1];
          pixels[i++]=pixel[2];
          pixels[i++]=pixel[3];
        }
      p+=GetPixelChannels(img);
    }
    if ((img->columns >= 8) && (img->columns <= 0x7ffff))
      {
        for (i=0; i < 4; i++)
          length=HDRWriteRunlengthPixels(img,&pixels[i*img->columns]);
      }
    else
      {
        count=WriteBlob(img,4*img->columns*sizeof(*pixels),pixels);
        if (count != (ssize_t) (4*img->columns*sizeof(*pixels)))
          break;
      }
    status=SetImageProgress(img,SaveImageTag,(MagickOffsetType) y,
      img->rows);
    if (status == MagickFalse)
      break;
  }
  pixels=(unsigned char *) RelinquishMagickMemory(pixels);
  (void) CloseBlob(img);
  return(MagickTrue);
}