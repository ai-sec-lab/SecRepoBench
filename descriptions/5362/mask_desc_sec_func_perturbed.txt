static void ImportGrayQuantum(const Image *img,QuantumInfo *quantum_info,
  const MagickSizeType number_pixels,const unsigned char *magick_restrict p,
  Quantum *magick_restrict q)
{
  QuantumAny
    range;

  register ssize_t
    x;

  ssize_t
    bit;

  unsigned int
    pixel;

  assert(img != (Image *) NULL);
  assert(img->signature == MagickCoreSignature);
  pixel=0;
  switch (quantum_info->depth)
  {
    case 1:
    {
      register Quantum
        black,
        white;

      black=0;
      white=QuantumRange;
      if (quantum_info->min_is_white != MagickFalse)
        {
          black=QuantumRange;
          white=0;
        }
      for (x=0; x < ((ssize_t) number_pixels-7); x+=8)
      {
        for (bit=0; bit < 8; bit++)
        {
          SetPixelGray(img,((*p) & (1 << (7-bit))) == 0 ? black : white,q);
          q+=GetPixelChannels(img);
        }
        p++;
      }
      for (bit=0; bit < (ssize_t) (number_pixels % 8); bit++)
      {
        SetPixelGray(img,((*p) & (0x01 << (7-bit))) == 0 ? black : white,q);
        q+=GetPixelChannels(img);
      }
      if (bit != 0)
        p++;
      break;
    }
    case 4:
    {
      register unsigned char
        pixel;

      range=GetQuantumRange(quantum_info->depth);
      for (x=0; x < ((ssize_t) number_pixels-1); x+=2)
      {
        pixel=(unsigned char) ((*p >> 4) & 0xf);
        SetPixelGray(img,ScaleAnyToQuantum(pixel,range),q);
        q+=GetPixelChannels(img);
        pixel=(unsigned char) ((*p) & 0xf);
        SetPixelGray(img,ScaleAnyToQuantum(pixel,range),q);
        p++;
        q+=GetPixelChannels(img);
      }
      for (bit=0; bit < (ssize_t) (number_pixels % 2); bit++)
      {
        pixel=(unsigned char) (*p++ >> 4);
        SetPixelGray(img,ScaleAnyToQuantum(pixel,range),q);
        q+=GetPixelChannels(img);
      }
      break;
    }
    case 8:
    {
      unsigned char
        pixel;

      if (quantum_info->min_is_white != MagickFalse)
        {
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            SetPixelGray(img,QuantumRange-ScaleCharToQuantum(pixel),q);
            SetPixelAlpha(img,OpaqueAlpha,q);
            p+=quantum_info->pad;
            q+=GetPixelChannels(img);
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        p=PushCharPixel(p,&pixel);
        SetPixelGray(img,ScaleCharToQuantum(pixel),q);
        SetPixelAlpha(img,OpaqueAlpha,q);
        p+=quantum_info->pad;
        q+=GetPixelChannels(img);
      }
      break;
    }
    case 10:
    {
      range=GetQuantumRange(quantum_info->depth);
      if (quantum_info->pack == MagickFalse)
        {
          if (img->endian == LSBEndian)
            {
              for (x=0; x < (ssize_t) (number_pixels-2); x+=3)
              {
                p=PushLongPixel(quantum_info->endian,p,&pixel);
                SetPixelGray(img,ScaleAnyToQuantum((pixel >> 22) & 0x3ff,
                  range),q);
                q+=GetPixelChannels(img);
                SetPixelGray(img,ScaleAnyToQuantum((pixel >> 12) & 0x3ff,
                  range),q);
                q+=GetPixelChannels(img);
                SetPixelGray(img,ScaleAnyToQuantum((pixel >> 2) & 0x3ff,
                  range),q);
                p+=quantum_info->pad;
                q+=GetPixelChannels(img);
              }
              if (x++ < (ssize_t) (number_pixels-1))
                {
                  p=PushLongPixel(quantum_info->endian,p,&pixel);
                  SetPixelGray(img,ScaleAnyToQuantum((pixel >> 22) & 0x3ff,
                    range),q);
                  q+=GetPixelChannels(img);
                }
              if (x++ < (ssize_t) number_pixels)
                {
                  SetPixelGray(img,ScaleAnyToQuantum((pixel >> 12) & 0x3ff,
                    range),q);
                  q+=GetPixelChannels(img);
                }
              break;
            }
          for (x=0; x < (ssize_t) (number_pixels-2); x+=3)
          {
            p=PushLongPixel(quantum_info->endian,p,&pixel);
            SetPixelGray(img,ScaleAnyToQuantum((pixel >> 2) & 0x3ff,range),
              q);
            q+=GetPixelChannels(img);
            SetPixelGray(img,ScaleAnyToQuantum((pixel >> 12) & 0x3ff,range),
              q);
            q+=GetPixelChannels(img);
            SetPixelGray(img,ScaleAnyToQuantum((pixel >> 22) & 0x3ff,range),
              q);
            p+=quantum_info->pad;
            q+=GetPixelChannels(img);
          }
          // The masked region handles the remaining pixels when the total number 
          // of pixels is not a multiple of three. It reads a long pixel from the 
          // source, applies a bitwise shift to extract a 10-bit gray value, scales 
          // it according to the quantum range, and then sets this gray value for 
          // the pixel. The operation is conditioned to ensure it does not exceed 
          // the number of pixels being processed.
          // <MASK>
          if (x++ < (ssize_t) number_pixels)
            {
              SetPixelGray(img,ScaleAnyToQuantum((pixel >> 12) & 0x3ff,
                range),q);
              q+=GetPixelChannels(img);
            }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        p=PushQuantumPixel(quantum_info,p,&pixel);
        SetPixelGray(img,ScaleAnyToQuantum(pixel,range),q);
        p+=quantum_info->pad;
        q+=GetPixelChannels(img);
      }
      break;
    }
    case 12:
    {
      range=GetQuantumRange(quantum_info->depth);
      if (quantum_info->pack == MagickFalse)
        {
          unsigned short
            pixel;

          for (x=0; x < (ssize_t) (number_pixels-1); x+=2)
          {
            p=PushShortPixel(quantum_info->endian,p,&pixel);
            SetPixelGray(img,ScaleAnyToQuantum((QuantumAny) (pixel >> 4),
              range),q);
            q+=GetPixelChannels(img);
            p=PushShortPixel(quantum_info->endian,p,&pixel);
            SetPixelGray(img,ScaleAnyToQuantum((QuantumAny) (pixel >> 4),
              range),q);
            p+=quantum_info->pad;
            q+=GetPixelChannels(img);
          }
          for (bit=0; bit < (ssize_t) (number_pixels % 2); bit++)
          {
            p=PushShortPixel(quantum_info->endian,p,&pixel);
            SetPixelGray(img,ScaleAnyToQuantum((QuantumAny) (pixel >> 4),
              range),q);
            p+=quantum_info->pad;
            q+=GetPixelChannels(img);
          }
          if (bit != 0)
            p++;
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        p=PushQuantumPixel(quantum_info,p,&pixel);
        SetPixelGray(img,ScaleAnyToQuantum(pixel,range),q);
        p+=quantum_info->pad;
        q+=GetPixelChannels(img);
      }
      break;
    }
    case 16:
    {
      unsigned short
        pixel;

      if (quantum_info->min_is_white != MagickFalse)
        {
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushShortPixel(quantum_info->endian,p,&pixel);
            SetPixelGray(img,QuantumRange-ScaleShortToQuantum(pixel),q);
            p+=quantum_info->pad;
            q+=GetPixelChannels(img);
          }
          break;
        }
      if (quantum_info->format == FloatingPointQuantumFormat)
        {
          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushShortPixel(quantum_info->endian,p,&pixel);
            SetPixelGray(img,ClampToQuantum(QuantumRange*
              HalfToSinglePrecision(pixel)),q);
            p+=quantum_info->pad;
            q+=GetPixelChannels(img);
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        p=PushShortPixel(quantum_info->endian,p,&pixel);
        SetPixelGray(img,ScaleShortToQuantum(pixel),q);
        p+=quantum_info->pad;
        q+=GetPixelChannels(img);
      }
      break;
    }
    case 32:
    {
      unsigned int
        pixel;

      if (quantum_info->format == FloatingPointQuantumFormat)
        {
          float
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushQuantumFloatPixel(quantum_info,p,&pixel);
            SetPixelGray(img,ClampToQuantum(pixel),q);
            p+=quantum_info->pad;
            q+=GetPixelChannels(img);
          }
          break;
        }
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        p=PushLongPixel(quantum_info->endian,p,&pixel);
        SetPixelGray(img,ScaleLongToQuantum(pixel),q);
        p+=quantum_info->pad;
        q+=GetPixelChannels(img);
      }
      break;
    }
    case 64:
    {
      if (quantum_info->format == FloatingPointQuantumFormat)
        {
          double
            pixel;

          for (x=0; x < (ssize_t) number_pixels; x++)
          {
            p=PushDoublePixel(quantum_info,p,&pixel);
            SetPixelGray(img,ClampToQuantum(pixel),q);
            p+=quantum_info->pad;
            q+=GetPixelChannels(img);
          }
          break;
        }
    }
    default:
    {
      range=GetQuantumRange(quantum_info->depth);
      for (x=0; x < (ssize_t) number_pixels; x++)
      {
        p=PushQuantumPixel(quantum_info,p,&pixel);
        SetPixelGray(img,ScaleAnyToQuantum(pixel,range),q);
        p+=quantum_info->pad;
        q+=GetPixelChannels(img);
      }
      break;
    }
  }
}