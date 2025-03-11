static Image *ReadTIFFImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
#define ThrowTIFFException(severity,message) \
{ \
  if (tiff_pixels != (unsigned char *) NULL) \
    tiff_pixels=(unsigned char *) RelinquishMagickMemory(tiff_pixels); \
  if (quantum_info != (QuantumInfo *) NULL) \
    quantum_info=DestroyQuantumInfo(quantum_info); \
  TIFFClose(tiff); \
  ThrowReaderException(severity,message); \
}

  const char
    *option;

  float
    *chromaticity,
    x_position,
    y_position,
    x_resolution,
    y_resolution;

  Image
    *tiff_image;

  int
    tiff_status;

  MagickBooleanType
    status;

  MagickSizeType
    number_pixels;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  register ssize_t
    i;

  size_t
    pad;

  ssize_t
    y;

  TIFF
    *tiff;

  TIFFMethodType
    method;

  uint16
    compress_tag,
    bits_per_sample,
    endian,
    extra_samples,
    interlace,
    max_sample_value,
    min_sample_value,
    orientation,
    pages,
    photometric,
    *sample_info,
    sample_format,
    samples_per_pixel,
    units,
    value;

  uint32
    height,
    rows_per_strip,
    width;

  unsigned char
    *tiff_pixels;

  /*
    Open image.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  tiff_image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,tiff_image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      tiff_image=DestroyImageList(tiff_image);
      return((Image *) NULL);
    }
  (void) SetMagickThreadValue(tiff_exception,exception);
  tiff=TIFFClientOpen(tiff_image->filename,"rb",(thandle_t) tiff_image,TIFFReadBlob,
    TIFFWriteBlob,TIFFSeekBlob,TIFFCloseBlob,TIFFGetBlobSize,TIFFMapBlob,
    TIFFUnmapBlob);
  if (tiff == (TIFF *) NULL)
    {
      tiff_image=DestroyImageList(tiff_image);
      return((Image *) NULL);
    }
  if (image_info->number_scenes != 0)
    {
      /*
        Generate blank images for subimage specification (e.g. image.tif[4].
        We need to check the number of directores because it is possible that
        the subimage(s) are stored in the photoshop profile.
      */
      if (image_info->scene < (size_t) TIFFNumberOfDirectories(tiff))
        {
          for (i=0; i < (ssize_t) image_info->scene; i++)
          {
            status=TIFFReadDirectory(tiff) != 0 ? MagickTrue : MagickFalse;
            if (status == MagickFalse)
              {
                TIFFClose(tiff);
                tiff_image=DestroyImageList(tiff_image);
                return((Image *) NULL);
              }
            AcquireNextImage(image_info,tiff_image,exception);
            if (GetNextImageInList(tiff_image) == (Image *) NULL)
              {
                TIFFClose(tiff);
                tiff_image=DestroyImageList(tiff_image);
                return((Image *) NULL);
              }
            tiff_image=SyncNextImageInList(tiff_image);
          }
      }
  }
  do
  {
DisableMSCWarning(4127)
    if (0 && (image_info->verbose != MagickFalse))
      TIFFPrintDirectory(tiff,stdout,MagickFalse);
RestoreMSCWarning
    if ((TIFFGetField(tiff,TIFFTAG_IMAGEWIDTH,&width) != 1) ||
        (TIFFGetField(tiff,TIFFTAG_IMAGELENGTH,&height) != 1) ||
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_COMPRESSION,&compress_tag) != 1) ||
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_FILLORDER,&endian) != 1) ||
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_PLANARCONFIG,&interlace) != 1) ||
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_SAMPLESPERPIXEL,&samples_per_pixel) != 1) ||
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_BITSPERSAMPLE,&bits_per_sample) != 1) ||
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_SAMPLEFORMAT,&sample_format) != 1) ||
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_MINSAMPLEVALUE,&min_sample_value) != 1) ||
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_MAXSAMPLEVALUE,&max_sample_value) != 1))
      {
        TIFFClose(tiff);
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
      }
    photometric=PHOTOMETRIC_RGB;
    (void) TIFFGetFieldDefaulted(tiff,TIFFTAG_PHOTOMETRIC,&photometric);
    if (sample_format == SAMPLEFORMAT_IEEEFP)
      (void) SetImageProperty(tiff_image,"quantum:format","floating-point",
        exception);
    switch (photometric)
    {
      case PHOTOMETRIC_MINISBLACK:
      {
        (void) SetImageProperty(tiff_image,"tiff:photometric","min-is-black",
          exception);
        break;
      }
      case PHOTOMETRIC_MINISWHITE:
      {
        (void) SetImageProperty(tiff_image,"tiff:photometric","min-is-white",
          exception);
        break;
      }
      case PHOTOMETRIC_PALETTE:
      {
        (void) SetImageProperty(tiff_image,"tiff:photometric","palette",exception);
        break;
      }
      case PHOTOMETRIC_RGB:
      {
        (void) SetImageProperty(tiff_image,"tiff:photometric","RGB",exception);
        break;
      }
      case PHOTOMETRIC_CIELAB:
      {
        (void) SetImageProperty(tiff_image,"tiff:photometric","CIELAB",exception);
        break;
      }
      case PHOTOMETRIC_LOGL:
      {
        (void) SetImageProperty(tiff_image,"tiff:photometric","CIE Log2(L)",
          exception);
        break;
      }
      case PHOTOMETRIC_LOGLUV:
      {
        (void) SetImageProperty(tiff_image,"tiff:photometric","LOGLUV",exception);
        break;
      }
#if defined(PHOTOMETRIC_MASK)
      case PHOTOMETRIC_MASK:
      {
        (void) SetImageProperty(tiff_image,"tiff:photometric","MASK",exception);
        break;
      }
#endif
      case PHOTOMETRIC_SEPARATED:
      {
        (void) SetImageProperty(tiff_image,"tiff:photometric","separated",exception);
        break;
      }
      case PHOTOMETRIC_YCBCR:
      {
        (void) SetImageProperty(tiff_image,"tiff:photometric","YCBCR",exception);
        break;
      }
      default:
      {
        (void) SetImageProperty(tiff_image,"tiff:photometric","unknown",exception);
        break;
      }
    }
    if (tiff_image->debug != MagickFalse)
      {
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Geometry: %ux%u",
          (unsigned int) width,(unsigned int) height);
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Interlace: %u",
          interlace);
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "Bits per sample: %u",bits_per_sample);
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "Min sample value: %u",min_sample_value);
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
          "Max sample value: %u",max_sample_value);
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Photometric "
          "interpretation: %s",GetImageProperty(tiff_image,"tiff:photometric",
          exception));
      }
    tiff_image->columns=(size_t) width;
    tiff_image->rows=(size_t) height;
    tiff_image->depth=(size_t) bits_per_sample;
    if (tiff_image->debug != MagickFalse)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Image depth: %.20g",
        (double) tiff_image->depth);
    tiff_image->endian=MSBEndian;
    if (endian == FILLORDER_LSB2MSB)
      tiff_image->endian=LSBEndian;
#if defined(MAGICKCORE_HAVE_TIFFISBIGENDIAN)
    if (TIFFIsBigEndian(tiff) == 0)
      {
        (void) SetImageProperty(tiff_image,"tiff:endian","lsb",exception);
        tiff_image->endian=LSBEndian;
      }
    else
      {
        (void) SetImageProperty(tiff_image,"tiff:endian","msb",exception);
        tiff_image->endian=MSBEndian;
      }
#endif
    if ((photometric == PHOTOMETRIC_MINISBLACK) ||
        (photometric == PHOTOMETRIC_MINISWHITE))
      SetImageColorspace(tiff_image,GRAYColorspace,exception);
    if (photometric == PHOTOMETRIC_SEPARATED)
      SetImageColorspace(tiff_image,CMYKColorspace,exception);
    if (photometric == PHOTOMETRIC_CIELAB)
      SetImageColorspace(tiff_image,LabColorspace,exception);
    TIFFGetProfiles(tiff,tiff_image,exception);
    TIFFGetProperties(tiff,tiff_image,exception);
    option=GetImageOption(image_info,"tiff:exif-properties");
    if (IsStringFalse(option) == MagickFalse) /* enabled by default */
      TIFFGetEXIFProperties(tiff,tiff_image,exception);
    (void) TIFFGetFieldDefaulted(tiff,TIFFTAG_SAMPLESPERPIXEL,
      &samples_per_pixel);
    if ((TIFFGetFieldDefaulted(tiff,TIFFTAG_XRESOLUTION,&x_resolution) == 1) &&
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_YRESOLUTION,&y_resolution) == 1))
      {
        tiff_image->resolution.x=x_resolution;
        tiff_image->resolution.y=y_resolution;
      }
    if (TIFFGetFieldDefaulted(tiff,TIFFTAG_RESOLUTIONUNIT,&units) == 1)
      {
        if (units == RESUNIT_INCH)
          tiff_image->units=PixelsPerInchResolution;
        if (units == RESUNIT_CENTIMETER)
          tiff_image->units=PixelsPerCentimeterResolution;
      }
    if ((TIFFGetFieldDefaulted(tiff,TIFFTAG_XPOSITION,&x_position) == 1) &&
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_YPOSITION,&y_position) == 1))
      {
        tiff_image->page.x=(ssize_t) ceil(x_position*tiff_image->resolution.x-0.5);
        tiff_image->page.y=(ssize_t) ceil(y_position*tiff_image->resolution.y-0.5);
      }
    if (TIFFGetFieldDefaulted(tiff,TIFFTAG_ORIENTATION,&orientation) == 1)
      tiff_image->orientation=(OrientationType) orientation;
    if (TIFFGetField(tiff,TIFFTAG_WHITEPOINT,&chromaticity) == 1)
      {
        if (chromaticity != (float *) NULL)
          {
            tiff_image->chromaticity.white_point.x=chromaticity[0];
            tiff_image->chromaticity.white_point.y=chromaticity[1];
          }
      }
    if (TIFFGetField(tiff,TIFFTAG_PRIMARYCHROMATICITIES,&chromaticity) == 1)
      {
        if (chromaticity != (float *) NULL)
          {
            tiff_image->chromaticity.red_primary.x=chromaticity[0];
            tiff_image->chromaticity.red_primary.y=chromaticity[1];
            tiff_image->chromaticity.green_primary.x=chromaticity[2];
            tiff_image->chromaticity.green_primary.y=chromaticity[3];
            tiff_image->chromaticity.blue_primary.x=chromaticity[4];
            tiff_image->chromaticity.blue_primary.y=chromaticity[5];
          }
      }
#if defined(MAGICKCORE_HAVE_TIFFISCODECCONFIGURED) || (TIFFLIB_VERSION > 20040919)
    if ((compress_tag != COMPRESSION_NONE) &&
        (TIFFIsCODECConfigured(compress_tag) == 0))
      {
        TIFFClose(tiff);
        ThrowReaderException(CoderError,"CompressNotSupported");
      }
#endif
    switch (compress_tag)
    {
      case COMPRESSION_NONE: tiff_image->compression=NoCompression; break;
      case COMPRESSION_CCITTFAX3: tiff_image->compression=FaxCompression; break;
      case COMPRESSION_CCITTFAX4: tiff_image->compression=Group4Compression; break;
      case COMPRESSION_JPEG:
      {
         tiff_image->compression=JPEGCompression;
#if defined(JPEG_SUPPORT)
         {
           char
             sampling_factor[MagickPathExtent];

           int
             tiff_status;

           uint16
             horizontal,
             vertical;

           tiff_status=TIFFGetField(tiff,TIFFTAG_YCBCRSUBSAMPLING,&horizontal,
             &vertical);
           if (tiff_status == 1)
             {
               (void) FormatLocaleString(sampling_factor,MagickPathExtent,
                 "%dx%d",horizontal,vertical);
               (void) SetImageProperty(tiff_image,"jpeg:sampling-factor",
                 sampling_factor,exception);
               (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                 "Sampling Factors: %s",sampling_factor);
             }
         }
#endif
        break;
      }
      case COMPRESSION_OJPEG: tiff_image->compression=JPEGCompression; break;
#if defined(COMPRESSION_LZMA)
      case COMPRESSION_LZMA: tiff_image->compression=LZMACompression; break;
#endif
      case COMPRESSION_LZW: tiff_image->compression=LZWCompression; break;
      case COMPRESSION_DEFLATE: tiff_image->compression=ZipCompression; break;
      case COMPRESSION_ADOBE_DEFLATE: tiff_image->compression=ZipCompression; break;
      default: tiff_image->compression=RLECompression; break;
    }
    quantum_info=(QuantumInfo *) NULL;
    if ((photometric == PHOTOMETRIC_PALETTE) &&
        (pow(2.0,1.0*bits_per_sample) <= MaxColormapSize))
      {
        size_t
          colors;

        colors=(size_t) GetQuantumRange(bits_per_sample)+1;
        if (AcquireImageColormap(tiff_image,colors,exception) == MagickFalse)
          {
            TIFFClose(tiff);
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          }
      }
    value=(unsigned short) tiff_image->scene;
    if (TIFFGetFieldDefaulted(tiff,TIFFTAG_PAGENUMBER,&value,&pages) == 1)
      tiff_image->scene=value;
    if (tiff_image->storage_class == PseudoClass)
      {
        int
          tiff_status;

        size_t
          range;

        uint16
          *blue_colormap,
          *green_colormap,
          *red_colormap;

        /*
          Initialize colormap.
        */
        tiff_status=TIFFGetField(tiff,TIFFTAG_COLORMAP,&red_colormap,
          &green_colormap,&blue_colormap);
        if (tiff_status == 1)
          {
            if ((red_colormap != (uint16 *) NULL) &&
                (green_colormap != (uint16 *) NULL) &&
                (blue_colormap != (uint16 *) NULL))
              {
                range=255;  /* might be old style 8-bit colormap */
                for (i=0; i < (ssize_t) tiff_image->colors; i++)
                  if ((red_colormap[i] >= 256) || (green_colormap[i] >= 256) ||
                      (blue_colormap[i] >= 256))
                    {
                      range=65535;
                      break;
                    }
                for (i=0; i < (ssize_t) tiff_image->colors; i++)
                {
                  tiff_image->colormap[i].red=ClampToQuantum(((double)
                    QuantumRange*red_colormap[i])/range);
                  tiff_image->colormap[i].green=ClampToQuantum(((double)
                    QuantumRange*green_colormap[i])/range);
                  tiff_image->colormap[i].blue=ClampToQuantum(((double)
                    QuantumRange*blue_colormap[i])/range);
                }
              }
          }
      }
    if (image_info->ping != MagickFalse)
      {
        if (image_info->number_scenes != 0)
          if (tiff_image->scene >= (image_info->scene+image_info->number_scenes-1))
            break;
        goto next_tiff_frame;
      }
    status=SetImageExtent(tiff_image,tiff_image->columns,tiff_image->rows,exception);
    if (status == MagickFalse)
      {
        TIFFClose(tiff);
        return(DestroyImageList(tiff_image));
      }
    /*
      Allocate memory for the image and pixel buffer.
    */
    tiff_pixels=(unsigned char *) NULL;
    quantum_info=AcquireQuantumInfo(image_info,tiff_image);
    if (quantum_info == (QuantumInfo *) NULL)
      ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
    if (sample_format == SAMPLEFORMAT_UINT)
      status=SetQuantumFormat(tiff_image,quantum_info,UnsignedQuantumFormat);
    if (sample_format == SAMPLEFORMAT_INT)
      status=SetQuantumFormat(tiff_image,quantum_info,SignedQuantumFormat);
    if (sample_format == SAMPLEFORMAT_IEEEFP)
      status=SetQuantumFormat(tiff_image,quantum_info,FloatingPointQuantumFormat);
    if (status == MagickFalse)
      ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
    status=MagickTrue;
    switch (photometric)
    {
      case PHOTOMETRIC_MINISBLACK:
      {
        quantum_info->min_is_white=MagickFalse;
        break;
      }
      case PHOTOMETRIC_MINISWHITE:
      {
        quantum_info->min_is_white=MagickTrue;
        break;
      }
      default:
        break;
    }
    tiff_status=TIFFGetFieldDefaulted(tiff,TIFFTAG_EXTRASAMPLES,&extra_samples,
      &sample_info);
    if (tiff_status == 1)
      {
        (void) SetImageProperty(tiff_image,"tiff:alpha","unspecified",exception);
        if (extra_samples == 0)
          {
            if ((samples_per_pixel == 4) && (photometric == PHOTOMETRIC_RGB))
              tiff_image->alpha_trait=BlendPixelTrait;
          }
        else
          for (i=0; i < extra_samples; i++)
          {
            tiff_image->alpha_trait=BlendPixelTrait;
            if (sample_info[i] == EXTRASAMPLE_ASSOCALPHA)
              {
                SetQuantumAlphaType(quantum_info,DisassociatedQuantumAlpha);
                (void) SetImageProperty(tiff_image,"tiff:alpha","associated",
                  exception);
              }
            else
              if (sample_info[i] == EXTRASAMPLE_UNASSALPHA)
                (void) SetImageProperty(tiff_image,"tiff:alpha","unassociated",
                  exception);
          }
      }
    method=ReadGenericMethod;
    if (TIFFGetField(tiff,TIFFTAG_ROWSPERSTRIP,&rows_per_strip) == 1)
      {
        char
          value[MagickPathExtent];

        method=ReadStripMethod;
        (void) FormatLocaleString(value,MagickPathExtent,"%u",
          (unsigned int) rows_per_strip);
        (void) SetImageProperty(tiff_image,"tiff:rows-per-strip",value,exception);
      }
    if (rows_per_strip > (tiff_image->columns*tiff_image->rows))
      ThrowTIFFException(CorruptImageError,"ImproperImageHeader");
    if ((samples_per_pixel >= 3) && (interlace == PLANARCONFIG_CONTIG))
      if ((tiff_image->alpha_trait == UndefinedPixelTrait) ||
          (samples_per_pixel >= 4))
        method=ReadRGBAMethod;
    if ((samples_per_pixel >= 4) && (interlace == PLANARCONFIG_SEPARATE))
      if ((tiff_image->alpha_trait == UndefinedPixelTrait) ||
          (samples_per_pixel >= 5))
        method=ReadCMYKAMethod;
    if ((photometric != PHOTOMETRIC_RGB) &&
        (photometric != PHOTOMETRIC_CIELAB) &&
        (photometric != PHOTOMETRIC_SEPARATED))
      method=ReadGenericMethod;
    if (tiff_image->storage_class == PseudoClass)
      method=ReadSingleSampleMethod;
    if ((photometric == PHOTOMETRIC_MINISBLACK) ||
        (photometric == PHOTOMETRIC_MINISWHITE))
      method=ReadSingleSampleMethod;
    if ((photometric != PHOTOMETRIC_SEPARATED) &&
        (interlace == PLANARCONFIG_SEPARATE) && (bits_per_sample < 64))
      method=ReadGenericMethod;
    if (tiff_image->compression == JPEGCompression)
      method=GetJPEGMethod(tiff_image,tiff,photometric,bits_per_sample,
        samples_per_pixel);
    if (compress_tag == COMPRESSION_JBIG)
      method=ReadStripMethod;
    if (TIFFIsTiled(tiff) != MagickFalse)
      method=ReadTileMethod;
    quantum_info->endian=LSBEndian;
    quantum_type=RGBQuantum;
    if (((MagickSizeType) TIFFScanlineSize(tiff)) > GetBlobSize(tiff_image))
      ThrowTIFFException(CorruptImageError,"InsufficientImageDataInFile");
// <MASK>
    switch (method)
    {
      case ReadSingleSampleMethod:
      {
        /*
          Convert TIFF image to PseudoClass MIFF image.
        */
        quantum_type=IndexQuantum;
        pad=(size_t) MagickMax((ssize_t) samples_per_pixel-1,0);
        if (tiff_image->alpha_trait != UndefinedPixelTrait)
          {
            if (tiff_image->storage_class != PseudoClass)
              {
                quantum_type=samples_per_pixel == 1 ? AlphaQuantum :
                  GrayAlphaQuantum;
                pad=(size_t) MagickMax((ssize_t) samples_per_pixel-2,0);
              }
            else
              {
                quantum_type=IndexAlphaQuantum;
                pad=(size_t) MagickMax((ssize_t) samples_per_pixel-2,0);
              }
          }
        else
          if (tiff_image->storage_class != PseudoClass)
            {
              quantum_type=GrayQuantum;
              pad=(size_t) MagickMax((ssize_t) samples_per_pixel-1,0);
            }
        status=SetQuantumPad(tiff_image,quantum_info,pad*pow(2,ceil(log(
          bits_per_sample)/log(2))));
        if (status == MagickFalse)
          ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
        for (y=0; y < (ssize_t) tiff_image->rows; y++)
        {
          int
            status;

          register Quantum
            *magick_restrict q;

          status=TIFFReadPixels(tiff,bits_per_sample,0,y,(char *) tiff_pixels);
          if (status == -1)
            break;
          q=QueueAuthenticPixels(tiff_image,0,y,tiff_image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          (void) ImportQuantumPixels(tiff_image,(CacheView *) NULL,quantum_info,
            quantum_type,tiff_pixels,exception);
          if (SyncAuthenticPixels(tiff_image,exception) == MagickFalse)
            break;
          if (tiff_image->previous == (Image *) NULL)
            {
              status=SetImageProgress(tiff_image,LoadImageTag,(MagickOffsetType) y,
                tiff_image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      case ReadRGBAMethod:
      {
        /*
          Convert TIFF image to DirectClass MIFF image.
        */
        pad=(size_t) MagickMax((size_t) samples_per_pixel-3,0);
        quantum_type=RGBQuantum;
        if (tiff_image->alpha_trait != UndefinedPixelTrait)
          {
            quantum_type=RGBAQuantum;
            pad=(size_t) MagickMax((size_t) samples_per_pixel-4,0);
          }
        if (tiff_image->colorspace == CMYKColorspace)
          {
            pad=(size_t) MagickMax((size_t) samples_per_pixel-4,0);
            quantum_type=CMYKQuantum;
            if (tiff_image->alpha_trait != UndefinedPixelTrait)
              {
                quantum_type=CMYKAQuantum;
                pad=(size_t) MagickMax((size_t) samples_per_pixel-5,0);
              }
          }
        status=SetQuantumPad(tiff_image,quantum_info,pad*((bits_per_sample+7) >> 3));
        if (status == MagickFalse)
          ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
        for (y=0; y < (ssize_t) tiff_image->rows; y++)
        {
          int
            status;

          register Quantum
            *magick_restrict q;

          status=TIFFReadPixels(tiff,bits_per_sample,0,y,(char *) tiff_pixels);
          if (status == -1)
            break;
          q=QueueAuthenticPixels(tiff_image,0,y,tiff_image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          (void) ImportQuantumPixels(tiff_image,(CacheView *) NULL,quantum_info,
            quantum_type,tiff_pixels,exception);
          if (SyncAuthenticPixels(tiff_image,exception) == MagickFalse)
            break;
          if (tiff_image->previous == (Image *) NULL)
            {
              status=SetImageProgress(tiff_image,LoadImageTag,(MagickOffsetType) y,
                tiff_image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      case ReadCMYKAMethod:
      {
        /*
          Convert TIFF image to DirectClass MIFF image.
        */
        for (i=0; i < (ssize_t) samples_per_pixel; i++)
        {
          for (y=0; y < (ssize_t) tiff_image->rows; y++)
          {
            register Quantum
              *magick_restrict q;

            int
              status;

            status=TIFFReadPixels(tiff,bits_per_sample,(tsample_t) i,y,(char *)
              tiff_pixels);
            if (status == -1)
              break;
            q=GetAuthenticPixels(tiff_image,0,y,tiff_image->columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            if (tiff_image->colorspace != CMYKColorspace)
              switch (i)
              {
                case 0: quantum_type=RedQuantum; break;
                case 1: quantum_type=GreenQuantum; break;
                case 2: quantum_type=BlueQuantum; break;
                case 3: quantum_type=AlphaQuantum; break;
                default: quantum_type=UndefinedQuantum; break;
              }
            else
              switch (i)
              {
                case 0: quantum_type=CyanQuantum; break;
                case 1: quantum_type=MagentaQuantum; break;
                case 2: quantum_type=YellowQuantum; break;
                case 3: quantum_type=BlackQuantum; break;
                case 4: quantum_type=AlphaQuantum; break;
                default: quantum_type=UndefinedQuantum; break;
              }
            (void) ImportQuantumPixels(tiff_image,(CacheView *) NULL,quantum_info,
              quantum_type,tiff_pixels,exception);
            if (SyncAuthenticPixels(tiff_image,exception) == MagickFalse)
              break;
          }
          if (tiff_image->previous == (Image *) NULL)
            {
              status=SetImageProgress(tiff_image,LoadImageTag,(MagickOffsetType) y,
                tiff_image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      case ReadYCCKMethod:
      {
        for (y=0; y < (ssize_t) tiff_image->rows; y++)
        {
          int
            status;

          register Quantum
            *magick_restrict q;

          register ssize_t
            x;

          unsigned char
            *p;

          status=TIFFReadPixels(tiff,bits_per_sample,0,y,(char *) tiff_pixels);
          if (status == -1)
            break;
          q=QueueAuthenticPixels(tiff_image,0,y,tiff_image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          p=tiff_pixels;
          for (x=0; x < (ssize_t) tiff_image->columns; x++)
          {
            SetPixelCyan(tiff_image,ScaleCharToQuantum(ClampYCC((double) *p+
              (1.402*(double) *(p+2))-179.456)),q);
            SetPixelMagenta(tiff_image,ScaleCharToQuantum(ClampYCC((double) *p-
              (0.34414*(double) *(p+1))-(0.71414*(double ) *(p+2))+
              135.45984)),q);
            SetPixelYellow(tiff_image,ScaleCharToQuantum(ClampYCC((double) *p+
              (1.772*(double) *(p+1))-226.816)),q);
            SetPixelBlack(tiff_image,ScaleCharToQuantum((unsigned char) *(p+3)),q);
            q+=GetPixelChannels(tiff_image);
            p+=4;
          }
          if (SyncAuthenticPixels(tiff_image,exception) == MagickFalse)
            break;
          if (tiff_image->previous == (Image *) NULL)
            {
              status=SetImageProgress(tiff_image,LoadImageTag,(MagickOffsetType) y,
                tiff_image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      case ReadStripMethod:
      {
        register uint32
          *p;

        /*
          Convert stripped TIFF image to DirectClass MIFF image.
        */
        i=0;
        p=(uint32 *) NULL;
        for (y=0; y < (ssize_t) tiff_image->rows; y++)
        {
          register ssize_t
            x;

          register Quantum
            *magick_restrict q;

          q=QueueAuthenticPixels(tiff_image,0,y,tiff_image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          if (i == 0)
            {
              if (TIFFReadRGBAStrip(tiff,(tstrip_t) y,(uint32 *) tiff_pixels) == 0)
                break;
              i=(ssize_t) MagickMin((ssize_t) rows_per_strip,(ssize_t)
                tiff_image->rows-y);
            }
          i--;
          p=((uint32 *) tiff_pixels)+tiff_image->columns*i;
          for (x=0; x < (ssize_t) tiff_image->columns; x++)
          {
            SetPixelRed(tiff_image,ScaleCharToQuantum((unsigned char)
              (TIFFGetR(*p))),q);
            SetPixelGreen(tiff_image,ScaleCharToQuantum((unsigned char)
              (TIFFGetG(*p))),q);
            SetPixelBlue(tiff_image,ScaleCharToQuantum((unsigned char)
              (TIFFGetB(*p))),q);
            if (tiff_image->alpha_trait != UndefinedPixelTrait)
              SetPixelAlpha(tiff_image,ScaleCharToQuantum((unsigned char)
                (TIFFGetA(*p))),q);
            p++;
            q+=GetPixelChannels(tiff_image);
          }
          if (SyncAuthenticPixels(tiff_image,exception) == MagickFalse)
            break;
          if (tiff_image->previous == (Image *) NULL)
            {
              status=SetImageProgress(tiff_image,LoadImageTag,(MagickOffsetType) y,
                tiff_image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      case ReadTileMethod:
      {
        register uint32
          *p;

        uint32
          *tile_pixels,
          columns,
          rows;

        /*
          Convert tiled TIFF image to DirectClass MIFF image.
        */
        if ((TIFFGetField(tiff,TIFFTAG_TILEWIDTH,&columns) != 1) ||
            (TIFFGetField(tiff,TIFFTAG_TILELENGTH,&rows) != 1))
          ThrowTIFFException(CoderError,"ImageIsNotTiled");
        if ((AcquireMagickResource(WidthResource,columns) == MagickFalse) ||
            (AcquireMagickResource(HeightResource,rows) == MagickFalse))
          ThrowTIFFException(ImageError,"WidthOrHeightExceedsLimit");
        (void) SetImageStorageClass(tiff_image,DirectClass,exception);
        number_pixels=(MagickSizeType) columns*rows;
        if (HeapOverflowSanityCheck(rows,sizeof(*tile_pixels)) != MagickFalse)
          ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
        tile_pixels=(uint32 *) AcquireQuantumMemory(columns,rows*
          sizeof(*tile_pixels));
        if (tile_pixels == (uint32 *) NULL)
          ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
        for (y=0; y < (ssize_t) tiff_image->rows; y+=rows)
        {
          register ssize_t
            x;

          register Quantum
            *magick_restrict q,
            *magick_restrict tile;

          size_t
            columns_remaining,
            rows_remaining;

          rows_remaining=tiff_image->rows-y;
          if ((ssize_t) (y+rows) < (ssize_t) tiff_image->rows)
            rows_remaining=rows;
          tile=QueueAuthenticPixels(tiff_image,0,y,tiff_image->columns,rows_remaining,
            exception);
          if (tile == (Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) tiff_image->columns; x+=columns)
          {
            size_t
              column,
              row;

            if (TIFFReadRGBATile(tiff,(uint32) x,(uint32) y,tile_pixels) == 0)
              break;
            columns_remaining=tiff_image->columns-x;
            if ((ssize_t) (x+columns) < (ssize_t) tiff_image->columns)
              columns_remaining=columns;
            p=tile_pixels+(rows-rows_remaining)*columns;
            q=tile+GetPixelChannels(tiff_image)*(tiff_image->columns*(rows_remaining-1)+
              x);
            for (row=rows_remaining; row > 0; row--)
            {
              if (tiff_image->alpha_trait != UndefinedPixelTrait)
                for (column=columns_remaining; column > 0; column--)
                {
                  SetPixelRed(tiff_image,ScaleCharToQuantum((unsigned char)
                    TIFFGetR(*p)),q);
                  SetPixelGreen(tiff_image,ScaleCharToQuantum((unsigned char)
                    TIFFGetG(*p)),q);
                  SetPixelBlue(tiff_image,ScaleCharToQuantum((unsigned char)
                    TIFFGetB(*p)),q);
                  SetPixelAlpha(tiff_image,ScaleCharToQuantum((unsigned char)
                    TIFFGetA(*p)),q);
                  p++;
                  q+=GetPixelChannels(tiff_image);
                }
              else
                for (column=columns_remaining; column > 0; column--)
                {
                  SetPixelRed(tiff_image,ScaleCharToQuantum((unsigned char)
                    TIFFGetR(*p)),q);
                  SetPixelGreen(tiff_image,ScaleCharToQuantum((unsigned char)
                    TIFFGetG(*p)),q);
                  SetPixelBlue(tiff_image,ScaleCharToQuantum((unsigned char)
                    TIFFGetB(*p)),q);
                  p++;
                  q+=GetPixelChannels(tiff_image);
                }
              p+=columns-columns_remaining;
              q-=GetPixelChannels(tiff_image)*(tiff_image->columns+columns_remaining);
            }
          }
          if (SyncAuthenticPixels(tiff_image,exception) == MagickFalse)
            break;
          if (tiff_image->previous == (Image *) NULL)
            {
              status=SetImageProgress(tiff_image,LoadImageTag,(MagickOffsetType) y,
                tiff_image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        tile_pixels=(uint32 *) RelinquishMagickMemory(tile_pixels);
        break;
      }
      case ReadGenericMethod:
      default:
      {
        MemoryInfo
          *pixel_info;

        register uint32
          *p;

        uint32
          *pixels;

        /*
          Convert TIFF image to DirectClass MIFF image.
        */
        number_pixels=(MagickSizeType) tiff_image->columns*tiff_image->rows;
        if (HeapOverflowSanityCheck(tiff_image->rows,sizeof(*pixels)) != MagickFalse)
          ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
        pixel_info=AcquireVirtualMemory(tiff_image->columns,tiff_image->rows*
          sizeof(uint32));
        if (pixel_info == (MemoryInfo *) NULL)
          ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
        pixels=(uint32 *) GetVirtualMemoryBlob(pixel_info);
        (void) TIFFReadRGBAImage(tiff,(uint32) tiff_image->columns,(uint32)
          tiff_image->rows,(uint32 *) pixels,0);
        /*
          Convert image to DirectClass pixel packets.
        */
        p=pixels+number_pixels-1;
        for (y=0; y < (ssize_t) tiff_image->rows; y++)
        {
          register ssize_t
            x;

          register Quantum
            *magick_restrict q;

          q=QueueAuthenticPixels(tiff_image,0,y,tiff_image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          q+=GetPixelChannels(tiff_image)*(tiff_image->columns-1);
          for (x=0; x < (ssize_t) tiff_image->columns; x++)
          {
            SetPixelRed(tiff_image,ScaleCharToQuantum((unsigned char)
              TIFFGetR(*p)),q);
            SetPixelGreen(tiff_image,ScaleCharToQuantum((unsigned char)
              TIFFGetG(*p)),q);
            SetPixelBlue(tiff_image,ScaleCharToQuantum((unsigned char)
              TIFFGetB(*p)),q);
            if (tiff_image->alpha_trait != UndefinedPixelTrait)
              SetPixelAlpha(tiff_image,ScaleCharToQuantum((unsigned char)
                TIFFGetA(*p)),q);
            p--;
            q-=GetPixelChannels(tiff_image);
          }
          if (SyncAuthenticPixels(tiff_image,exception) == MagickFalse)
            break;
          if (tiff_image->previous == (Image *) NULL)
            {
              status=SetImageProgress(tiff_image,LoadImageTag,(MagickOffsetType) y,
                tiff_image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        pixel_info=RelinquishVirtualMemory(pixel_info);
        break;
      }
    }
    tiff_pixels=(unsigned char *) RelinquishMagickMemory(tiff_pixels);
    SetQuantumImageType(tiff_image,quantum_type);
  next_tiff_frame:
    if (quantum_info != (QuantumInfo *) NULL)
      quantum_info=DestroyQuantumInfo(quantum_info);
    if (photometric == PHOTOMETRIC_CIELAB)
      DecodeLabImage(tiff_image,exception);
    if ((photometric == PHOTOMETRIC_LOGL) ||
        (photometric == PHOTOMETRIC_MINISBLACK) ||
        (photometric == PHOTOMETRIC_MINISWHITE))
      {
        tiff_image->type=GrayscaleType;
        if (bits_per_sample == 1)
          tiff_image->type=BilevelType;
      }
    /*
      Proceed to next image.
    */
    if (image_info->number_scenes != 0)
      if (tiff_image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    status=TIFFReadDirectory(tiff) != 0 ? MagickTrue : MagickFalse;
    if (status != MagickFalse)
      {
        /*
          Allocate next image structure.
        */
        AcquireNextImage(image_info,tiff_image,exception);
        if (GetNextImageInList(tiff_image) == (Image *) NULL)
          {
            tiff_image=DestroyImageList(tiff_image);
            return((Image *) NULL);
          }
        tiff_image=SyncNextImageInList(tiff_image);
        status=SetImageProgress(tiff_image,LoadImagesTag,tiff_image->scene-1,
          tiff_image->scene);
        if (status == MagickFalse)
          break;
      }
  } while (status != MagickFalse);
  TIFFClose(tiff);
  TIFFReadPhotoshopLayers(tiff_image,image_info,exception);
  if (image_info->number_scenes != 0)
    {
      if (image_info->scene >= GetImageListLength(tiff_image))
        {
          /* Subimage was not found in the Photoshop layer */
          tiff_image=DestroyImageList(tiff_image);
          return((Image *)NULL);
        }
    }
  return(GetFirstImageInList(tiff_image));
}