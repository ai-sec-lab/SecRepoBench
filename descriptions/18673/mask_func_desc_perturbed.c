static void ImportCMYKAQuantum(const Image *image,QuantumInfo *quantum_info,
  const MagickSizeType number_pixels,const unsigned char *magick_restrict p,
  Quantum *magick_restrict q,ExceptionInfo *exception)
{
  QuantumAny
    range;

  register ssize_t
    x;

  unsigned int
    colorvalue;

  if (image->colorspace != CMYKColorspace)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
        "ColorSeparatedImageRequired","`%s'",image->filename);
      return;
    }
  switch (quantum_info->depth)
  {
    case 8:
    {
      unsigned char
        colorvalue;

      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        p=PushCharPixel(p,&colorvalue);
        SetPixelRed(image,ScaleCharToQuantum(colorvalue),q);
        p=PushCharPixel(p,&colorvalue);
        SetPixelGreen(image,ScaleCharToQuantum(colorvalue),q);
        p=PushCharPixel(p,&colorvalue);
        SetPixelBlue(image,ScaleCharToQuantum(colorvalue),q);
        p=PushCharPixel(p,&colorvalue);
        SetPixelBlack(image,ScaleCharToQuantum(colorvalue),q);
        p=PushCharPixel(p,&colorvalue);
        SetPixelAlpha(image,ScaleCharToQuantum(colorvalue),q);
        p+=quantum_info->pad;
        q+=GetPixelChannels(image);
      }
      break;
    }
    case 16:
    {
      unsigned short
        colorvalue;

      if (quantum_info->format == FloatingPointQuantumFormat)
        {
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushShortPixel(quantum_info->endian,p,&colorvalue);
            SetPixelRed(image,ClampToQuantum(QuantumRange*
              HalfToSinglePrecision(colorvalue)),q);
            p=PushShortPixel(quantum_info->endian,p,&colorvalue);
            SetPixelGreen(image,ClampToQuantum(QuantumRange*
              HalfToSinglePrecision(colorvalue)),q);
            p=PushShortPixel(quantum_info->endian,p,&colorvalue);
            SetPixelBlue(image,ClampToQuantum(QuantumRange*
              HalfToSinglePrecision(colorvalue)),q);
            p=PushShortPixel(quantum_info->endian,p,&colorvalue);
            SetPixelBlack(image,ClampToQuantum(QuantumRange*
              HalfToSinglePrecision(colorvalue)),q);
            p=PushShortPixel(quantum_info->endian,p,&colorvalue);
            SetPixelAlpha(image,ClampToQuantum(QuantumRange*
              HalfToSinglePrecision(colorvalue)),q);
            p+=quantum_info->pad;
            q+=GetPixelChannels(image);
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        p=PushShortPixel(quantum_info->endian,p,&colorvalue);
        SetPixelRed(image,ScaleShortToQuantum(colorvalue),q);
        p=PushShortPixel(quantum_info->endian,p,&colorvalue);
        SetPixelGreen(image,ScaleShortToQuantum(colorvalue),q);
        p=PushShortPixel(quantum_info->endian,p,&colorvalue);
        SetPixelBlue(image,ScaleShortToQuantum(colorvalue),q);
        p=PushShortPixel(quantum_info->endian,p,&colorvalue);
        SetPixelBlack(image,ScaleShortToQuantum(colorvalue),q);
        p=PushShortPixel(quantum_info->endian,p,&colorvalue);
        SetPixelAlpha(image,ScaleShortToQuantum(colorvalue),q);
        p+=quantum_info->pad;
        q+=GetPixelChannels(image);
      }
      break;
    }
    case 32:
    {
      unsigned int
        colorvalue;

      if (quantum_info->format == FloatingPointQuantumFormat)
        {
          float
            colorvalue;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumFloatPixel(quantum_info,p,&colorvalue);
            SetPixelRed(image,ClampToQuantum(colorvalue),q);
            p=PushQuantumFloatPixel(quantum_info,p,&colorvalue);
            SetPixelGreen(image,ClampToQuantum(colorvalue),q);
            p=PushQuantumFloatPixel(quantum_info,p,&colorvalue);
            SetPixelBlue(image,ClampToQuantum(colorvalue),q);
            p=PushQuantumFloatPixel(quantum_info,p,&colorvalue);
            SetPixelBlack(image,ClampToQuantum(colorvalue),q);
            p=PushQuantumFloatPixel(quantum_info,p,&colorvalue);
            SetPixelAlpha(image,ClampToQuantum(colorvalue),q);
            p+=quantum_info->pad;
            q+=GetPixelChannels(image);
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        p=PushLongPixel(quantum_info->endian,p,&colorvalue);
        SetPixelRed(image,ScaleLongToQuantum(colorvalue),q);
        p=PushLongPixel(quantum_info->endian,p,&colorvalue);
        SetPixelGreen(image,ScaleLongToQuantum(colorvalue),q);
        p=PushLongPixel(quantum_info->endian,p,&colorvalue);
        SetPixelBlue(image,ScaleLongToQuantum(colorvalue),q);
        p=PushLongPixel(quantum_info->endian,p,&colorvalue);
        SetPixelBlack(image,ScaleLongToQuantum(colorvalue),q);
        p=PushLongPixel(quantum_info->endian,p,&colorvalue);
        SetPixelAlpha(image,ScaleLongToQuantum(colorvalue),q);
        p+=quantum_info->pad;
        q+=GetPixelChannels(image);
      }
      break;
    }
    case 64:
    {
      if (quantum_info->format == FloatingPointQuantumFormat)
        {
          double
            colorvalue;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            // For each pixel, extract the red, green, blue, black, and alpha channel values.
            // Convert the extracted double precision floating-point channel values to quantum values.
            // Set the corresponding quantum values for red, green, blue, black, and alpha channels in the image.
            // Adjust the pointer to account for padding and move to the next pixel.
            // <MASK>
          }
          break;
        }
    }
    default:
    {
      range=GetQuantumRange(quantum_info->depth);
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        p=PushQuantumPixel(quantum_info,p,&colorvalue);
        SetPixelRed(image,ScaleAnyToQuantum(colorvalue,range),q);
        p=PushQuantumPixel(quantum_info,p,&colorvalue);
        SetPixelGreen(image,ScaleAnyToQuantum(colorvalue,range),q);
        p=PushQuantumPixel(quantum_info,p,&colorvalue);
        SetPixelBlue(image,ScaleAnyToQuantum(colorvalue,range),q);
        p=PushQuantumPixel(quantum_info,p,&colorvalue);
        SetPixelBlack(image,ScaleAnyToQuantum(colorvalue,range),q);
        p=PushQuantumPixel(quantum_info,p,&colorvalue);
        SetPixelAlpha(image,ScaleAnyToQuantum(colorvalue,range),q);
        q+=GetPixelChannels(image);
      }
      break;
    }
  }
}