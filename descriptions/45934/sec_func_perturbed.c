static Image *ReadMIFFImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
#define BZipMaxExtent(x)  ((x)+((x)/100)+600)
#define LZMAMaxExtent(x)  ((x)+((x)/3)+128)
#define ThrowMIFFException(exception,message) \
{ \
  if (quantum_info != (QuantumInfo *) NULL) \
    quantum_info=DestroyQuantumInfo(quantum_info); \
  if (compress_pixels != (unsigned char *) NULL) \
    compress_pixels=(unsigned char *) RelinquishMagickMemory(compress_pixels); \
  ThrowReaderException((exception),(message)); \
}
#define ZipMaxExtent(x)  ((x)+(((x)+7) >> 3)+(((x)+63) >> 6)+11)

#if defined(MAGICKCORE_BZLIB_DELEGATE)
  bz_stream
    bzip_info;
#endif

  char
    id[MagickPathExtent],
    keyword[MagickPathExtent],
    *options;

  double
    version;

  GeometryInfo
    geometry_info;

  Image
    *output_image;

  int
    c;

  LinkedListInfo
    *profiles;

#if defined(MAGICKCORE_LZMA_DELEGATE)
  lzma_stream
    initialize_lzma = LZMA_STREAM_INIT,
    lzma_info = LZMA_STREAM_INIT;

  lzma_allocator
    allocator;
#endif

  MagickBooleanType
    status;

  PixelInfo
    pixel;

  MagickStatusType
    flags;

  QuantumFormatType
    quantum_format;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  ssize_t
    i;

  size_t
    compress_extent,
    extent,
    length,
    packet_size;

  ssize_t
    count;

  unsigned char
    *compress_pixels,
    *pixels;

  size_t
    colors;

  ssize_t
    y;

#if defined(MAGICKCORE_ZLIB_DELEGATE)
  z_stream
    zip_info;
#endif

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
  output_image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,output_image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      output_image=DestroyImageList(output_image);
      return((Image *) NULL);
    }
  /*
    Decode image header;  header terminates one character beyond a ':'.
  */
  c=ReadBlobByte(output_image);
  if (c == EOF)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  *id='\0';
  compress_pixels=(unsigned char *) NULL;
  quantum_info=(QuantumInfo *) NULL;
  (void) memset(keyword,0,sizeof(keyword));
  version=0.0;
  (void) version;
  do
  {
    /*
      Decode image header;  header terminates one character beyond a ':'.
    */
    SetGeometryInfo(&geometry_info);
    length=MagickPathExtent;
    options=AcquireString((char *) NULL);
    quantum_format=UndefinedQuantumFormat;
    profiles=(LinkedListInfo *) NULL;
    colors=0;
    output_image->depth=8UL;
    output_image->compression=NoCompression;
    while ((isgraph((int) ((unsigned char) c)) != 0) && (c != (int) ':'))
    {
      char
        *p;

      if (c == (int) '{')
        {
          char
            *comment;

          /*
            Read comment-- any text between { }.
          */
          length=MagickPathExtent;
          comment=AcquireString((char *) NULL);
          for (p=comment; comment != (char *) NULL; p++)
          {
            c=ReadBlobByte(output_image);
            if (c == (int) '\\')
              c=ReadBlobByte(output_image);
            else
              if ((c == EOF) || (c == (int) '}'))
                break;
            if ((size_t) (p-comment+1) >= length)
              {
                *p='\0';
                length<<=1;
                comment=(char *) ResizeQuantumMemory(comment,
                  OverAllocateMemory(length+MagickPathExtent),sizeof(*comment));
                if (comment == (char *) NULL)
                  break;
                p=comment+strlen(comment);
              }
            *p=(char) c;
          }
          if (comment == (char *) NULL)
            {
              options=DestroyString(options);
              ThrowMIFFException(ResourceLimitError,"MemoryAllocationFailed");
            }
          *p='\0';
          (void) SetImageProperty(output_image,"comment",comment,exception);
          comment=DestroyString(comment);
          c=ReadBlobByte(output_image);
        }
      else
        if (isalnum((int) ((unsigned char) c)) != MagickFalse)
          {
            /*
              Get the keyword.
            */
            length=MagickPathExtent-1;
            p=keyword;
            do
            {
              if (c == (int) '=')
                break;
              if ((size_t) (p-keyword) < (MagickPathExtent-1))
                *p++=(char) c;
              c=ReadBlobByte(output_image);
            } while (c != EOF);
            *p='\0';
            p=options;
            while ((isspace((int) ((unsigned char) c)) != 0) && (c != EOF))
              c=ReadBlobByte(output_image);
            if (c == (int) '=')
              {
                /*
                  Get the keyword value.
                */
                c=ReadBlobByte(output_image);
                while ((c != (int) '}') && (c != EOF))
                {
                  if ((size_t) (p-options+1) >= length)
                    {
                      *p='\0';
                      length<<=1;
                      options=(char *) ResizeQuantumMemory(options,length+
                        MagickPathExtent,sizeof(*options));
                      if (options == (char *) NULL)
                        break;
                      p=options+strlen(options);
                    }
                  *p++=(char) c;
                  c=ReadBlobByte(output_image);
                  if (c == '\\')
                    {
                      c=ReadBlobByte(output_image);
                      if (c == (int) '}')
                        {
                          *p++=(char) c;
                          c=ReadBlobByte(output_image);
                        }
                    }
                  if (*options != '{')
                    if (isspace((int) ((unsigned char) c)) != 0)
                      break;
                }
                if (options == (char *) NULL)
                  ThrowMIFFException(ResourceLimitError,
                    "MemoryAllocationFailed");
              }
            *p='\0';
            if (*options == '{')
              (void) CopyMagickString(options,options+1,strlen(options));
            /*
              Assign a value to the specified keyword.
            */
            switch (*keyword)
            {
              case 'a':
              case 'A':
              {
                if (LocaleCompare(keyword,"alpha-trait") == 0)
                  {
                    ssize_t
                      alpha_trait;

                    alpha_trait=ParseCommandOption(MagickPixelTraitOptions,
                      MagickFalse,options);
                    if (alpha_trait < 0)
                      break;
                    output_image->alpha_trait=(PixelTrait) alpha_trait;
                    break;
                  }
                (void) SetImageProperty(output_image,keyword,options,exception);
                break;
              }
              case 'b':
              case 'B':
              {
                if (LocaleCompare(keyword,"background-color") == 0)
                  {
                    (void) QueryColorCompliance(options,AllCompliance,
                      &output_image->background_color,exception);
                    break;
                  }
                if (LocaleCompare(keyword,"blue-primary") == 0)
                  {
                    flags=ParseGeometry(options,&geometry_info);
                    output_image->chromaticity.blue_primary.x=geometry_info.rho;
                    output_image->chromaticity.blue_primary.y=geometry_info.sigma;
                    if ((flags & SigmaValue) == 0)
                      output_image->chromaticity.blue_primary.y=
                        output_image->chromaticity.blue_primary.x;
                    break;
                  }
                if (LocaleCompare(keyword,"border-color") == 0)
                  {
                    (void) QueryColorCompliance(options,AllCompliance,
                      &output_image->border_color,exception);
                    break;
                  }
                (void) SetImageProperty(output_image,keyword,options,exception);
                break;
              }
              case 'c':
              case 'C':
              {
                if (LocaleCompare(keyword,"class") == 0)
                  {
                    ssize_t
                      storage_class;

                    storage_class=ParseCommandOption(MagickClassOptions,
                      MagickFalse,options);
                    if (storage_class < 0)
                      break;
                    output_image->storage_class=(ClassType) storage_class;
                    break;
                  }
                if (LocaleCompare(keyword,"colors") == 0)
                  {
                    colors=StringToUnsignedLong(options);
                    break;
                  }
                if (LocaleCompare(keyword,"colorspace") == 0)
                  {
                    ssize_t
                      colorspace;

                    colorspace=ParseCommandOption(MagickColorspaceOptions,
                      MagickFalse,options);
                    if (colorspace < 0)
                      break;
                    output_image->colorspace=(ColorspaceType) colorspace;
                    break;
                  }
                if (LocaleCompare(keyword,"compression") == 0)
                  {
                    ssize_t
                      compression;

                    compression=ParseCommandOption(MagickCompressOptions,
                      MagickFalse,options);
                    if (compression < 0)
                      break;
                    output_image->compression=(CompressionType) compression;
                    break;
                  }
                if (LocaleCompare(keyword,"columns") == 0)
                  {
                    output_image->columns=StringToUnsignedLong(options);
                    break;
                  }
                (void) SetImageProperty(output_image,keyword,options,exception);
                break;
              }
              case 'd':
              case 'D':
              {
                if (LocaleCompare(keyword,"delay") == 0)
                  {
                    output_image->delay=StringToUnsignedLong(options);
                    break;
                  }
                if (LocaleCompare(keyword,"depth") == 0)
                  {
                    output_image->depth=StringToUnsignedLong(options);
                    break;
                  }
                if (LocaleCompare(keyword,"dispose") == 0)
                  {
                    ssize_t
                      dispose;

                    dispose=ParseCommandOption(MagickDisposeOptions,MagickFalse,
                      options);
                    if (dispose < 0)
                      break;
                    output_image->dispose=(DisposeType) dispose;
                    break;
                  }
                (void) SetImageProperty(output_image,keyword,options,exception);
                break;
              }
              case 'e':
              case 'E':
              {
                if (LocaleCompare(keyword,"endian") == 0)
                  {
                    ssize_t
                      endian;

                    endian=ParseCommandOption(MagickEndianOptions,MagickFalse,
                      options);
                    if (endian < 0)
                      break;
                    output_image->endian=(EndianType) endian;
                    break;
                  }
                (void) SetImageProperty(output_image,keyword,options,exception);
                break;
              }
              case 'g':
              case 'G':
              {
                if (LocaleCompare(keyword,"gamma") == 0)
                  {
                    output_image->gamma=StringToDouble(options,(char **) NULL);
                    break;
                  }
                if (LocaleCompare(keyword,"gravity") == 0)
                  {
                    ssize_t
                      gravity;

                    gravity=ParseCommandOption(MagickGravityOptions,MagickFalse,
                      options);
                    if (gravity < 0)
                      break;
                    output_image->gravity=(GravityType) gravity;
                    break;
                  }
                if (LocaleCompare(keyword,"green-primary") == 0)
                  {
                    flags=ParseGeometry(options,&geometry_info);
                    output_image->chromaticity.green_primary.x=geometry_info.rho;
                    output_image->chromaticity.green_primary.y=geometry_info.sigma;
                    if ((flags & SigmaValue) == 0)
                      output_image->chromaticity.green_primary.y=
                        output_image->chromaticity.green_primary.x;
                    break;
                  }
                (void) SetImageProperty(output_image,keyword,options,exception);
                break;
              }
              case 'i':
              case 'I':
              {
                if (LocaleCompare(keyword,"id") == 0)
                  {
                    if (*id != '\0')
                      ThrowMIFFException(CorruptImageError,
                        "ImproperImageHeader");
                    (void) CopyMagickString(id,options,MagickPathExtent);
                    break;
                  }
                if (LocaleCompare(keyword,"iterations") == 0)
                  {
                    output_image->iterations=StringToUnsignedLong(options);
                    break;
                  }
                (void) SetImageProperty(output_image,keyword,options,exception);
                break;
              }
              case 'm':
              case 'M':
              {
                if (LocaleCompare(keyword,"matte") == 0)
                  {
                    ssize_t
                      matte;

                    matte=ParseCommandOption(MagickBooleanOptions,MagickFalse,
                      options);
                    if (matte < 0)
                      break;
                    output_image->alpha_trait=matte == 0 ? UndefinedPixelTrait :
                      BlendPixelTrait;
                    break;
                  }
                if (LocaleCompare(keyword,"mattecolor") == 0)
                  {
                    (void) QueryColorCompliance(options,AllCompliance,
                      &output_image->matte_color,exception);
                    break;
                  }
                if (LocaleCompare(keyword,"montage") == 0)
                  {
                    (void) CloneString(&output_image->montage,options);
                    break;
                  }
                (void) SetImageProperty(output_image,keyword,options,exception);
                break;
              }
              case 'o':
              case 'O':
              {
                if (LocaleCompare(keyword,"orientation") == 0)
                  {
                    ssize_t
                      orientation;

                    orientation=ParseCommandOption(MagickOrientationOptions,
                      MagickFalse,options);
                    if (orientation < 0)
                      break;
                    output_image->orientation=(OrientationType) orientation;
                    break;
                  }
                (void) SetImageProperty(output_image,keyword,options,exception);
                break;
              }
              case 'p':
              case 'P':
              {
                if (LocaleCompare(keyword,"page") == 0)
                  {
                    char
                      *geometry;

                    geometry=GetPageGeometry(options);
                    (void) ParseAbsoluteGeometry(geometry,&output_image->page);
                    geometry=DestroyString(geometry);
                    break;
                  }
                if (LocaleCompare(keyword,"pixel-intensity") == 0)
                  {
                    ssize_t
                      intensity;

                    intensity=ParseCommandOption(MagickPixelIntensityOptions,
                      MagickFalse,options);
                    if (intensity < 0)
                      break;
                    output_image->intensity=(PixelIntensityMethod) intensity;
                    break;
                  }
                if (LocaleCompare(keyword,"profile") == 0)
                  {
                    if (profiles == (LinkedListInfo *) NULL)
                      profiles=NewLinkedList(0);
                    (void) AppendValueToLinkedList(profiles,
                      AcquireString(options));
                    break;
                  }
                (void) SetImageProperty(output_image,keyword,options,exception);
                break;
              }
              case 'q':
              case 'Q':
              {
                if (LocaleCompare(keyword,"quality") == 0)
                  {
                    output_image->quality=StringToUnsignedLong(options);
                    break;
                  }
                if ((LocaleCompare(keyword,"quantum-format") == 0) ||
                    (LocaleCompare(keyword,"quantum:format") == 0))
                  {
                    ssize_t
                      format;

                    format=ParseCommandOption(MagickQuantumFormatOptions,
                      MagickFalse,options);
                    if (format < 0)
                      break;
                    quantum_format=(QuantumFormatType) format;
                    break;
                  }
                (void) SetImageProperty(output_image,keyword,options,exception);
                break;
              }
              case 'r':
              case 'R':
              {
                if (LocaleCompare(keyword,"red-primary") == 0)
                  {
                    flags=ParseGeometry(options,&geometry_info);
                    output_image->chromaticity.red_primary.x=geometry_info.rho;
                    output_image->chromaticity.red_primary.y=geometry_info.sigma;
                    if ((flags & SigmaValue) == 0)
                      output_image->chromaticity.red_primary.y=
                        output_image->chromaticity.red_primary.x;
                    break;
                  }
                if (LocaleCompare(keyword,"rendering-intent") == 0)
                  {
                    ssize_t
                      rendering_intent;

                    rendering_intent=ParseCommandOption(MagickIntentOptions,
                      MagickFalse,options);
                    if (rendering_intent < 0)
                      break;
                    output_image->rendering_intent=(RenderingIntent) rendering_intent;
                    break;
                  }
                if (LocaleCompare(keyword,"resolution") == 0)
                  {
                    flags=ParseGeometry(options,&geometry_info);
                    output_image->resolution.x=geometry_info.rho;
                    output_image->resolution.y=geometry_info.sigma;
                    if ((flags & SigmaValue) == 0)
                      output_image->resolution.y=output_image->resolution.x;
                    break;
                  }
                if (LocaleCompare(keyword,"rows") == 0)
                  {
                    output_image->rows=StringToUnsignedLong(options);
                    break;
                  }
                (void) SetImageProperty(output_image,keyword,options,exception);
                break;
              }
              case 's':
              case 'S':
              {
                if (LocaleCompare(keyword,"scene") == 0)
                  {
                    output_image->scene=StringToUnsignedLong(options);
                    break;
                  }
                (void) SetImageProperty(output_image,keyword,options,exception);
                break;
              }
              case 't':
              case 'T':
              {
                if (LocaleCompare(keyword,"ticks-per-second") == 0)
                  {
                    output_image->ticks_per_second=(ssize_t) StringToLong(options);
                    break;
                  }
                if (LocaleCompare(keyword,"tile-offset") == 0)
                  {
                    char
                      *geometry;

                    geometry=GetPageGeometry(options);
                    (void) ParseAbsoluteGeometry(geometry,&output_image->tile_offset);
                    geometry=DestroyString(geometry);
                    break;
                  }
                if (LocaleCompare(keyword,"type") == 0)
                  {
                    ssize_t
                      type;

                    type=ParseCommandOption(MagickTypeOptions,MagickFalse,
                      options);
                    if (type < 0)
                      break;
                    output_image->type=(ImageType) type;
                    break;
                  }
                (void) SetImageProperty(output_image,keyword,options,exception);
                break;
              }
              case 'u':
              case 'U':
              {
                if (LocaleCompare(keyword,"units") == 0)
                  {
                    ssize_t
                      units;

                    units=ParseCommandOption(MagickResolutionOptions,
                      MagickFalse,options);
                    if (units < 0)
                      break;
                    output_image->units=(ResolutionType) units;
                    break;
                  }
                (void) SetImageProperty(output_image,keyword,options,exception);
                break;
              }
              case 'v':
              case 'V':
              {
                if (LocaleCompare(keyword,"version") == 0)
                  {
                    version=StringToDouble(options,(char **) NULL);
                    break;
                  }
                (void) SetImageProperty(output_image,keyword,options,exception);
                break;
              }
              case 'w':
              case 'W':
              {
                if (LocaleCompare(keyword,"white-point") == 0)
                  {
                    flags=ParseGeometry(options,&geometry_info);
                    output_image->chromaticity.white_point.x=geometry_info.rho;
                    output_image->chromaticity.white_point.y=geometry_info.sigma;
                    if ((flags & SigmaValue) == 0)
                      output_image->chromaticity.white_point.y=
                        output_image->chromaticity.white_point.x;
                    break;
                  }
                (void) SetImageProperty(output_image,keyword,options,exception);
                break;
              }
              default:
              {
                (void) SetImageProperty(output_image,keyword,options,exception);
                break;
              }
            }
          }
        else
          c=ReadBlobByte(output_image);
      while (isspace((int) ((unsigned char) c)) != 0)
        c=ReadBlobByte(output_image);
    }
    options=DestroyString(options);
    (void) ReadBlobByte(output_image);
    /*
      Verify that required image information is defined.
    */
    if ((LocaleCompare(id,"ImageMagick") != 0) ||
        (output_image->storage_class == UndefinedClass) ||
        (output_image->compression == UndefinedCompression) ||
        (output_image->colorspace == UndefinedColorspace) ||
        (output_image->columns == 0) || (output_image->rows == 0) ||
        (output_image->depth == 0) || (output_image->depth > 64))
      {
        if (profiles != (LinkedListInfo *) NULL)
          profiles=DestroyLinkedList(profiles,RelinquishMagickMemory);
        if (output_image->previous == (Image *) NULL)
          ThrowMIFFException(CorruptImageError,"ImproperImageHeader");
        DeleteImageFromList(&output_image);
        (void) ThrowMagickException(exception,GetMagickModule(),
          CorruptImageError,"ImproperImageHeader","`%s'",output_image->filename);
        break;
      }
    if (output_image->montage != (char *) NULL)
      {
        char
          *p;

        /*
          Image directory.
        */
        extent=MagickPathExtent;
        output_image->directory=AcquireString((char *) NULL);
        p=output_image->directory;
        length=0;
        do
        {
          *p='\0';
          if ((length+MagickPathExtent) >= extent)
            {
              /*
                Allocate more memory for the image directory.
              */
              extent<<=1;
              output_image->directory=(char *) ResizeQuantumMemory(output_image->directory,
                extent+MagickPathExtent,sizeof(*output_image->directory));
              if (output_image->directory == (char *) NULL)
                ThrowMIFFException(CorruptImageError,"UnableToReadImageData");
              p=output_image->directory+length;
            }
          c=ReadBlobByte(output_image);
          if (c == EOF)
            break;
          *p++=(char) c;
          length++;
        } while (c != (int) '\0');
      }
    if (profiles != (LinkedListInfo *) NULL)
      {
        const char
          *name;

        StringInfo
          *profile;

        /*
          Read image profiles.
        */
        ResetLinkedListIterator(profiles);
        name=(const char *) GetNextValueInLinkedList(profiles);
        while (name != (const char *) NULL)
        {
          length=ReadBlobMSBLong(output_image);
          if ((length == 0) || ((MagickSizeType) length > GetBlobSize(output_image)))
            break;
          profile=AcquireStringInfo(length);
          if (profile == (StringInfo *) NULL)
            break;
          count=ReadBlob(output_image,length,GetStringInfoDatum(profile));
          if (count != (ssize_t) length)
            {
              profile=DestroyStringInfo(profile);
              break;
            }
          status=SetImageProfile(output_image,name,profile,exception);
          profile=DestroyStringInfo(profile);
          if (status == MagickFalse)
            break;
          name=(const char *) GetNextValueInLinkedList(profiles);
        }
        profiles=DestroyLinkedList(profiles,RelinquishMagickMemory);
      }
    output_image->depth=GetImageQuantumDepth(output_image,MagickFalse);
    if (output_image->storage_class == PseudoClass)
      {
        unsigned char
          *colormap;

        /*
          Create image colormap.
        */
        packet_size=(size_t) (3UL*output_image->depth/8UL);
        if ((MagickSizeType) colors > GetBlobSize(output_image))
          ThrowMIFFException(CorruptImageError,"InsufficientImageDataInFile");
        if (((MagickSizeType) packet_size*colors) > GetBlobSize(output_image))
          ThrowMIFFException(CorruptImageError,"InsufficientImageDataInFile");
        status=AcquireImageColormap(output_image,colors != 0 ? colors : 256,exception);
        if (status == MagickFalse)
          ThrowMIFFException(ResourceLimitError,"MemoryAllocationFailed");
        if (colors != 0)
          {
            const unsigned char
              *p;

            /*
              Read image colormap from file.
            */
            colormap=(unsigned char *) AcquireQuantumMemory(output_image->colors,
              packet_size*sizeof(*colormap));
            if (colormap == (unsigned char *) NULL)
              ThrowMIFFException(ResourceLimitError,"MemoryAllocationFailed");
            count=ReadBlob(output_image,packet_size*output_image->colors,colormap);
            p=colormap;
            switch (output_image->depth)
            {
              case 8:
              {
                unsigned char
                  char_pixel;

                for (i=0; i < (ssize_t) output_image->colors; i++)
                {
                  p=PushCharPixel(p,&char_pixel);
                  output_image->colormap[i].red=(MagickRealType)
                    ScaleCharToQuantum(char_pixel);
                  p=PushCharPixel(p,&char_pixel);
                  output_image->colormap[i].green=(MagickRealType)
                    ScaleCharToQuantum(char_pixel);
                  p=PushCharPixel(p,&char_pixel);
                  output_image->colormap[i].blue=(MagickRealType)
                    ScaleCharToQuantum(char_pixel);
                }
                break;
              }
              case 16:
              {
                unsigned short
                  short_pixel;

                for (i=0; i < (ssize_t) output_image->colors; i++)
                {
                  p=PushShortPixel(MSBEndian,p,&short_pixel);
                  output_image->colormap[i].red=(MagickRealType)
                    ScaleShortToQuantum(short_pixel);
                  p=PushShortPixel(MSBEndian,p,&short_pixel);
                  output_image->colormap[i].green=(MagickRealType)
                    ScaleShortToQuantum(short_pixel);
                  p=PushShortPixel(MSBEndian,p,&short_pixel);
                  output_image->colormap[i].blue=(MagickRealType)
                    ScaleShortToQuantum(short_pixel);
                }
                break;
              }
              case 32:
              default:
              {
                unsigned int
                  long_pixel;

                for (i=0; i < (ssize_t) output_image->colors; i++)
                {
                  p=PushLongPixel(MSBEndian,p,&long_pixel);
                  output_image->colormap[i].red=(MagickRealType)
                    ScaleLongToQuantum(long_pixel);
                  p=PushLongPixel(MSBEndian,p,&long_pixel);
                  output_image->colormap[i].green=(MagickRealType)
                    ScaleLongToQuantum(long_pixel);
                  p=PushLongPixel(MSBEndian,p,&long_pixel);
                  output_image->colormap[i].blue=(MagickRealType)
                    ScaleLongToQuantum(long_pixel);
                }
                break;
              }
            }
            colormap=(unsigned char *) RelinquishMagickMemory(colormap);
          }
      }
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (output_image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    status=SetImageExtent(output_image,output_image->columns,output_image->rows,exception);
    if (status == MagickFalse)
      return(DestroyImageList(output_image));
    status=ResetImagePixels(output_image,exception);
    if (status == MagickFalse)
      return(DestroyImageList(output_image));
    /*
      Allocate image pixels.
    */
    quantum_info=AcquireQuantumInfo(image_info,output_image);
    if (quantum_info == (QuantumInfo *) NULL)
      ThrowMIFFException(ResourceLimitError,"MemoryAllocationFailed");
    if (quantum_format != UndefinedQuantumFormat)
      {
        status=SetQuantumFormat(output_image,quantum_info,quantum_format);
        if (status == MagickFalse)
          ThrowMIFFException(ResourceLimitError,"MemoryAllocationFailed");
      }
    packet_size=(size_t) (quantum_info->depth/8);
    if (output_image->storage_class == DirectClass)
      packet_size=(size_t) (3*quantum_info->depth/8);
    if (IsGrayColorspace(output_image->colorspace) != MagickFalse)
      packet_size=quantum_info->depth/8;
    if (output_image->alpha_trait != UndefinedPixelTrait)
      packet_size+=quantum_info->depth/8;
    if (output_image->colorspace == CMYKColorspace)
      packet_size+=quantum_info->depth/8;
    if (output_image->compression == RLECompression)
      packet_size++;
    compress_extent=MagickMax(MagickMax(BZipMaxExtent(packet_size*
      output_image->columns),LZMAMaxExtent(packet_size*output_image->columns)),
      ZipMaxExtent(packet_size*output_image->columns));
    if (compress_extent < (packet_size*output_image->columns))
      ThrowMIFFException(ResourceLimitError,"MemoryAllocationFailed");
    compress_pixels=(unsigned char *) AcquireQuantumMemory(compress_extent,
      sizeof(*compress_pixels));
    if (compress_pixels == (unsigned char *) NULL)
      ThrowMIFFException(ResourceLimitError,"MemoryAllocationFailed");
    /*
      Read image pixels.
    */
    quantum_type=RGBQuantum;
    if (output_image->alpha_trait != UndefinedPixelTrait)
      quantum_type=RGBAQuantum;
    if (output_image->colorspace == CMYKColorspace)
      {
        quantum_type=CMYKQuantum;
        if (output_image->alpha_trait != UndefinedPixelTrait)
          quantum_type=CMYKAQuantum;
      }
    if (IsGrayColorspace(output_image->colorspace) != MagickFalse)
      {
        quantum_type=GrayQuantum;
        if (output_image->alpha_trait != UndefinedPixelTrait)
          quantum_type=GrayAlphaQuantum;
      }
    if (output_image->storage_class == PseudoClass)
      {
        quantum_type=IndexQuantum;
        if (output_image->alpha_trait != UndefinedPixelTrait)
          quantum_type=IndexAlphaQuantum;
      }
    status=MagickTrue;
    GetPixelInfo(output_image,&pixel);
#if defined(MAGICKCORE_BZLIB_DELEGATE)
   (void) memset(&bzip_info,0,sizeof(bzip_info));
#endif
#if defined(MAGICKCORE_LZMA_DELEGATE)
    (void) memset(&allocator,0,sizeof(allocator));
#endif
#if defined(MAGICKCORE_ZLIB_DELEGATE)
    (void) memset(&zip_info,0,sizeof(zip_info));
#endif
    switch (output_image->compression)
    {
#if defined(MAGICKCORE_BZLIB_DELEGATE)
      case BZipCompression:
      {
        int
          code;

        bzip_info.bzalloc=AcquireBZIPMemory;
        bzip_info.bzfree=RelinquishBZIPMemory;
        bzip_info.opaque=(void *) output_image;
        code=BZ2_bzDecompressInit(&bzip_info,(int) image_info->verbose,
          MagickFalse);
        if (code != BZ_OK)
          status=MagickFalse;
        break;
      }
#endif
#if defined(MAGICKCORE_LZMA_DELEGATE)
      case LZMACompression:
      {
        int
          code;

        allocator.alloc=AcquireLZMAMemory;
        allocator.free=RelinquishLZMAMemory;
        allocator.opaque=(void *) output_image;
        lzma_info=initialize_lzma;
        lzma_info.allocator=(&allocator);
        code=lzma_auto_decoder(&lzma_info,(uint64_t) -1,0);
        if (code != LZMA_OK)
          status=MagickFalse;
        break;
      }
#endif
#if defined(MAGICKCORE_ZLIB_DELEGATE)
      case LZWCompression:
      case ZipCompression:
      {
        int
          code;

        zip_info.zalloc=AcquireZIPMemory;
        zip_info.zfree=RelinquishZIPMemory;
        zip_info.opaque=(voidpf) output_image;
        code=inflateInit(&zip_info);
        if (code != Z_OK)
          status=MagickFalse;
        break;
      }
#endif
      case RLECompression:
        break;
      default:
        break;
    }
    pixels=(unsigned char *) GetQuantumPixels(quantum_info);
    length=0;
    for (y=0; y < (ssize_t) output_image->rows; y++)
    {
      ssize_t
        x;

      Quantum
        *magick_restrict q;

      if (status == MagickFalse)
        break;
      q=QueueAuthenticPixels(output_image,0,y,output_image->columns,1,exception);
      if (q == (Quantum *) NULL)
        break;
      extent=0;
      switch (output_image->compression)
      {
#if defined(MAGICKCORE_BZLIB_DELEGATE)
        case BZipCompression:
        {
          bzip_info.next_out=(char *) pixels;
          bzip_info.avail_out=(unsigned int) (packet_size*output_image->columns);
          do
          {
            int
              code;

            if (bzip_info.avail_in == 0)
              {
                bzip_info.next_in=(char *) compress_pixels;
                length=(size_t) BZipMaxExtent(packet_size*output_image->columns);
                if (version != 0.0)
                  length=(size_t) ReadBlobMSBLong(output_image);
                if (length <= compress_extent)
                  bzip_info.avail_in=(unsigned int) ReadBlob(output_image,length,
                    (unsigned char *) bzip_info.next_in);
                if ((length > compress_extent) ||
                    ((size_t) bzip_info.avail_in != length))
                  {
                    (void) BZ2_bzDecompressEnd(&bzip_info);
                    ThrowMIFFException(CorruptImageError,
                      "UnableToReadImageData");
                  }
              }
            code=BZ2_bzDecompress(&bzip_info);
            if ((code != BZ_OK) && (code != BZ_STREAM_END))
              {
                status=MagickFalse;
                break;
              }
            if (code == BZ_STREAM_END)
              break;
          } while (bzip_info.avail_out != 0);
          extent=ImportQuantumPixels(output_image,(CacheView *) NULL,quantum_info,
            quantum_type,pixels,exception);
          break;
        }
#endif
#if defined(MAGICKCORE_LZMA_DELEGATE)
        case LZMACompression:
        {
          lzma_info.next_out=pixels;
          lzma_info.avail_out=packet_size*output_image->columns;
          do
          {
            int
              code;

            if (lzma_info.avail_in == 0)
              {
                lzma_info.next_in=compress_pixels;
                length=(size_t) ReadBlobMSBLong(output_image);
                if (length <= compress_extent)
                  lzma_info.avail_in=(unsigned int) ReadBlob(output_image,length,
                    (unsigned char *) lzma_info.next_in);
                if ((length > compress_extent) ||
                    (lzma_info.avail_in != length))
                  {
                    lzma_end(&lzma_info);
                    ThrowMIFFException(CorruptImageError,
                      "UnableToReadImageData");
                  }
              }
            code=lzma_code(&lzma_info,LZMA_RUN);
            if ((code != LZMA_OK) && (code != LZMA_STREAM_END))
              {
                status=MagickFalse;
                break;
              }
            if (code == LZMA_STREAM_END)
              break;
          } while (lzma_info.avail_out != 0);
          extent=ImportQuantumPixels(output_image,(CacheView *) NULL,quantum_info,
            quantum_type,pixels,exception);
          break;
        }
#endif
#if defined(MAGICKCORE_ZLIB_DELEGATE)
        case LZWCompression:
        case ZipCompression:
        {
          zip_info.next_out=pixels;
          zip_info.avail_out=(uInt) (packet_size*output_image->columns);
          do
          {
            int
              code;

            if (zip_info.avail_in == 0)
              {
                zip_info.next_in=compress_pixels;
                length=(size_t) ZipMaxExtent(packet_size*output_image->columns);
                if (version != 0.0)
                  length=(size_t) ReadBlobMSBLong(output_image);
                if (length <= compress_extent)
                  zip_info.avail_in=(unsigned int) ReadBlob(output_image,length,
                    zip_info.next_in);
                if ((length > compress_extent) ||
                    ((size_t) zip_info.avail_in != length))
                  {
                    (void) inflateEnd(&zip_info);
                    ThrowMIFFException(CorruptImageError,
                      "UnableToReadImageData");
                  }
              }
            code=inflate(&zip_info,Z_SYNC_FLUSH);
            if ((code != Z_OK) && (code != Z_STREAM_END))
              {
                status=MagickFalse;
                break;
              }
            if (code == Z_STREAM_END)
              break;
          } while (zip_info.avail_out != 0);
          extent=ImportQuantumPixels(output_image,(CacheView *) NULL,quantum_info,
            quantum_type,pixels,exception);
          break;
        }
#endif
        case RLECompression:
        {
          for (x=0; x < (ssize_t) output_image->columns; x++)
          {
            if (length == 0)
              {
                count=ReadBlob(output_image,packet_size,pixels);
                if (count != (ssize_t) packet_size)
                  ThrowMIFFException(CorruptImageError,"UnableToReadImageData");
                PushRunlengthPacket(output_image,pixels,&length,&pixel,exception);
              }
            length--;
            if (output_image->storage_class == PseudoClass)
              SetPixelIndex(output_image,ClampToQuantum(pixel.index),q);
            else
              {
                SetPixelRed(output_image,ClampToQuantum(pixel.red),q);
                SetPixelGreen(output_image,ClampToQuantum(pixel.green),q);
                SetPixelBlue(output_image,ClampToQuantum(pixel.blue),q);
                if (output_image->colorspace == CMYKColorspace)
                  SetPixelBlack(output_image,ClampToQuantum(pixel.black),q);
              }
            if (output_image->alpha_trait != UndefinedPixelTrait)
              SetPixelAlpha(output_image,ClampToQuantum(pixel.alpha),q);
            q+=GetPixelChannels(output_image);
          }
          extent=(size_t) x;
          break;
        }
        default:
        {
          const void
            *stream;

          stream=ReadBlobStream(output_image,packet_size*output_image->columns,pixels,&count);
          if (count != (ssize_t) (packet_size*output_image->columns))
            ThrowMIFFException(CorruptImageError,"UnableToReadImageData");
          extent=ImportQuantumPixels(output_image,(CacheView *) NULL,quantum_info,
            quantum_type,(unsigned char *) stream,exception);
          break;
        }
      }
      if (extent < output_image->columns)
        break;
      if (SyncAuthenticPixels(output_image,exception) == MagickFalse)
        break;
    }
    SetQuantumImageType(output_image,quantum_type);
    switch (output_image->compression)
    {
#if defined(MAGICKCORE_BZLIB_DELEGATE)
      case BZipCompression:
      {
        int
          code;

        if (version == 0.0)
          {
            MagickOffsetType
              offset;

            offset=SeekBlob(output_image,-((MagickOffsetType) bzip_info.avail_in),
              SEEK_CUR);
            if (offset < 0)
              {
                (void) BZ2_bzDecompressEnd(&bzip_info);
                ThrowMIFFException(CorruptImageError,"ImproperImageHeader");
              }
          }
        code=BZ2_bzDecompressEnd(&bzip_info);
        if (code != BZ_OK)
          status=MagickFalse;
        break;
      }
#endif
#if defined(MAGICKCORE_LZMA_DELEGATE)
      case LZMACompression:
      {
        int
          code;

        code=lzma_code(&lzma_info,LZMA_FINISH);
        if ((code != LZMA_STREAM_END) && (code != LZMA_OK))
          status=MagickFalse;
        lzma_end(&lzma_info);
        break;
      }
#endif
#if defined(MAGICKCORE_ZLIB_DELEGATE)
      case LZWCompression:
      case ZipCompression:
      {
        int
          code;

        if (version == 0.0)
          {
            MagickOffsetType
              offset;

            offset=SeekBlob(output_image,-((MagickOffsetType) zip_info.avail_in),
              SEEK_CUR);
            if (offset < 0)
              {
                (void) inflateEnd(&zip_info);
                ThrowMIFFException(CorruptImageError,"ImproperImageHeader");
              }
          }
        code=inflateEnd(&zip_info);
        if (code != Z_OK)
          status=MagickFalse;
        break;
      }
#endif
      default:
        break;
    }
    quantum_info=DestroyQuantumInfo(quantum_info);
    compress_pixels=(unsigned char *) RelinquishMagickMemory(compress_pixels);
    if (((y != (ssize_t) output_image->rows)) || (status == MagickFalse))
      {
        output_image=DestroyImageList(output_image);
        return((Image *) NULL);
      }
    if (EOFBlob(output_image) != MagickFalse)
      {
        ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
          output_image->filename);
        break;
      }
    /*
      Proceed to next image.
    */
    if (image_info->number_scenes != 0)
      if (output_image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    do
    {
      c=ReadBlobByte(output_image);
    } while ((isgraph((int) ((unsigned char) c)) == 0) && (c != EOF));
    if ((c != EOF) && ((c == 'i') || (c == 'I')))
      {
        /*
          Allocate next image structure.
        */
        AcquireNextImage(image_info,output_image,exception);
        if (GetNextImageInList(output_image) == (Image *) NULL)
          {
            status=MagickFalse;
            break;
          }
        output_image=SyncNextImageInList(output_image);
        status=SetImageProgress(output_image,LoadImagesTag,TellBlob(output_image),
          GetBlobSize(output_image));
        if (status == MagickFalse)
          break;
      }
  } while (c != EOF && ((c == 'i') || (c == 'I')));
  (void) CloseBlob(output_image);
  if (status == MagickFalse)
    return(DestroyImageList(output_image));
  return(GetFirstImageInList(output_image));
}