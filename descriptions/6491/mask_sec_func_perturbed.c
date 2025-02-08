static Image *ReadHDRImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    format[MagickPathExtent],
    keyword[MagickPathExtent],
    tag[MagickPathExtent],
    value[MagickPathExtent];

  double
    gamma;

  Image
    *image;

  int
    c;

  MagickBooleanType
    status,
    value_expected;

  register Quantum
    *q;

  register ssize_t
    i,
    x;

  register unsigned char
    *p;

  ssize_t
    count,
    y;

  unsigned char
    *end,
    pixel[4],
    *pixels;

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
  /*
    Decode image header.
  */
  image->columns=0;
  image->rows=0;
  *format='\0';
  c=ReadBlobByte(image);
  if (c == EOF)
    {
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  while (isgraph(c) && (image->columns == 0) && (image->rows == 0))
  {
    if (c == (int) '#')
      {
        char
          *comment;

        register char
          *p;

        size_t
          length;

        /*
          Read comment-- any text between # and end-of-line.
        */
        length=MagickPathExtent;
        comment=AcquireString((char *) NULL);
        for (p=comment; comment != (char *) NULL; p++)
        {
          c=ReadBlobByte(image);
          if ((c == EOF) || (c == (int) '\n'))
            break;
          if ((size_t) (p-comment+1) >= length)
            {
              *p='\0';
              length<<=1;
              comment=(char *) ResizeQuantumMemory(comment,length+
                MagickPathExtent,sizeof(*comment));
              if (comment == (char *) NULL)
                break;
              p=comment+strlen(comment);
            }
          *p=(char) c;
        }
        if (comment == (char *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        *p='\0';
        (void) SetImageProperty(image,"comment",comment,exception);
        comment=DestroyString(comment);
        c=ReadBlobByte(image);
      }
    else
      if (isalnum(c) == MagickFalse)
        c=ReadBlobByte(image);
      else
        {
          register char
            *p;

          /*
            Determine a keyword and its value.
          */
          p=keyword;
          do
          {
            if ((size_t) (p-keyword) < (MagickPathExtent-1))
              *p++=c;
            c=ReadBlobByte(image);
          } while (isalnum(c) || (c == '_'));
          *p='\0';
          value_expected=MagickFalse;
          while ((isspace((int) ((unsigned char) c)) != 0) || (c == '='))
          {
            if (c == '=')
              value_expected=MagickTrue;
            c=ReadBlobByte(image);
          }
          if (LocaleCompare(keyword,"Y") == 0)
            value_expected=MagickTrue;
          if (value_expected == MagickFalse)
            continue;
          p=value;
          while ((c != '\n') && (c != '\0') && (c != EOF))
          {
            if ((size_t) (p-value) < (MagickPathExtent-1))
              *p++=c;
            c=ReadBlobByte(image);
          }
          *p='\0';
          /*
            Assign a value to the specified keyword.
          */
          switch (*keyword)
          {
            case 'F':
            case 'f':
            {
              if (LocaleCompare(keyword,"format") == 0)
                {
                  (void) CopyMagickString(format,value,MagickPathExtent);
                  break;
                }
              (void) FormatLocaleString(tag,MagickPathExtent,"hdr:%s",keyword);
              (void) SetImageProperty(image,tag,value,exception);
              break;
            }
            case 'G':
            case 'g':
            {
              if (LocaleCompare(keyword,"gamma") == 0)
                {
                  image->gamma=StringToDouble(value,(char **) NULL);
                  break;
                }
              (void) FormatLocaleString(tag,MagickPathExtent,"hdr:%s",keyword);
              (void) SetImageProperty(image,tag,value,exception);
              break;
            }
            case 'P':
            case 'p':
            {
              if (LocaleCompare(keyword,"primaries") == 0)
                {
                  float
                    chromaticity[6],
                    white_point[2];

                  int
                    count;

                  count=sscanf(value,"%g %g %g %g %g %g %g %g",&chromaticity[0],
                    &chromaticity[1],&chromaticity[2],&chromaticity[3],
                    &chromaticity[4],&chromaticity[5],&white_point[0],
                    &white_point[1]);
                  if (count == 8)
                    {
                      image->chromaticity.red_primary.x=chromaticity[0];
                      image->chromaticity.red_primary.y=chromaticity[1];
                      image->chromaticity.green_primary.x=chromaticity[2];
                      image->chromaticity.green_primary.y=chromaticity[3];
                      image->chromaticity.blue_primary.x=chromaticity[4];
                      image->chromaticity.blue_primary.y=chromaticity[5];
                      image->chromaticity.white_point.x=white_point[0],
                      image->chromaticity.white_point.y=white_point[1];
                    }
                  break;
                }
              (void) FormatLocaleString(tag,MagickPathExtent,"hdr:%s",keyword);
              (void) SetImageProperty(image,tag,value,exception);
              break;
            }
            case 'Y':
            case 'y':
            {
              char
                target[] = "Y";

              if (strcmp(keyword,target) == 0)
                {
                  int
                    imgheight,
                    width;

                  if (sscanf(value,"%d +X %d",&imgheight,&width) == 2)
                    {
                      image->columns=(size_t) width;
                      image->rows=(size_t) imgheight;
                    }
                  break;
                }
              (void) FormatLocaleString(tag,MagickPathExtent,"hdr:%s",keyword);
              (void) SetImageProperty(image,tag,value,exception);
              break;
            }
            default:
            {
              (void) FormatLocaleString(tag,MagickPathExtent,"hdr:%s",keyword);
              (void) SetImageProperty(image,tag,value,exception);
              break;
            }
          }
        }
    if ((image->columns == 0) && (image->rows == 0))
      while (isspace((int) ((unsigned char) c)) != 0)
        c=ReadBlobByte(image);
  }
  if ((LocaleCompare(format,"32-bit_rle_rgbe") != 0) &&
      (LocaleCompare(format,"32-bit_rle_xyze") != 0))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  if ((image->columns == 0) || (image->rows == 0))
    ThrowReaderException(CorruptImageError,"NegativeOrZeroImageSize");
  (void) SetImageColorspace(image,RGBColorspace,exception);
  if (LocaleCompare(format,"32-bit_rle_xyze") == 0)
    (void) SetImageColorspace(image,XYZColorspace,exception);
  image->compression=(image->columns < 8) || (image->columns > 0x7ffff) ?
    NoCompression : RLECompression;
  if (image_info->ping != MagickFalse)
    {
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  /*
    Read RGBE (red+green+blue+exponent) pixels.
  */
  // <MASK>
  pixels=(unsigned char *) RelinquishMagickMemory(pixels);
  if (EOFBlob(image) != MagickFalse)
    ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}