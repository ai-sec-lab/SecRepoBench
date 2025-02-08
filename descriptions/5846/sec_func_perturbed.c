static Image *ReadDPXImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    magick[4],
    value[MagickPathExtent];

  DPXInfo
    dpxinfo;

  Image
    *image;

  MagickBooleanType
    status;

  MagickOffsetType
    offset;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  register ssize_t
    i;

  size_t
    extent,
    samples_per_pixel;

  ssize_t
    count,
    n,
    row,
    y;

  unsigned char
    component_type;

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
    Read DPX file header.
  */
  offset=0;
  count=ReadBlob(image,4,(unsigned char *) magick);
  offset+=count;
  if ((count != 4) || ((LocaleNCompare(magick,"SDPX",4) != 0) &&
      (LocaleNCompare((char *) magick,"XPDS",4) != 0)))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  image->endian=LSBEndian;
  if (LocaleNCompare(magick,"SDPX",4) == 0)
    image->endian=MSBEndian;
  (void) ResetMagickMemory(&dpxinfo,0,sizeof(dpxinfo));
  dpxinfo.file.image_offset=ReadBlobLong(image);
  offset+=4;
  offset+=ReadBlob(image,sizeof(dpxinfo.file.version),(unsigned char *)
    dpxinfo.file.version);
  (void) FormatImageProperty(image,"dpx:file.version","%.8s",dpxinfo.file.version);
  dpxinfo.file.file_size=ReadBlobLong(image);
  offset+=4;
  dpxinfo.file.ditto_key=ReadBlobLong(image);
  offset+=4;
  if (dpxinfo.file.ditto_key != ~0U)
    (void) FormatImageProperty(image,"dpx:file.ditto.key","%u",
      dpxinfo.file.ditto_key);
  dpxinfo.file.generic_size=ReadBlobLong(image);
  offset+=4;
  dpxinfo.file.industry_size=ReadBlobLong(image);
  offset+=4;
  dpxinfo.file.user_size=ReadBlobLong(image);
  offset+=4;
  offset+=ReadBlob(image,sizeof(dpxinfo.file.filename),(unsigned char *)
    dpxinfo.file.filename);
  (void) FormatImageProperty(image,"dpx:file.filename","%.100s",
    dpxinfo.file.filename);
  (void) FormatImageProperty(image,"document","%.100s",dpxinfo.file.filename);
  offset+=ReadBlob(image,sizeof(dpxinfo.file.timestamp),(unsigned char *)
    dpxinfo.file.timestamp);
  if (*dpxinfo.file.timestamp != '\0')
    (void) FormatImageProperty(image,"dpx:file.timestamp","%.24s",
      dpxinfo.file.timestamp);
  offset+=ReadBlob(image,sizeof(dpxinfo.file.creator),(unsigned char *)
    dpxinfo.file.creator);
  if (*dpxinfo.file.creator != '\0')
    {
      (void) FormatImageProperty(image,"dpx:file.creator","%.100s",
        dpxinfo.file.creator);
      (void) FormatImageProperty(image,"software","%.100s",dpxinfo.file.creator);
    }
  offset+=ReadBlob(image,sizeof(dpxinfo.file.project),(unsigned char *)
    dpxinfo.file.project);
  if (*dpxinfo.file.project != '\0')
    {
      (void) FormatImageProperty(image,"dpx:file.project","%.200s",
        dpxinfo.file.project);
      (void) FormatImageProperty(image,"comment","%.100s",dpxinfo.file.project);
    }
  offset+=ReadBlob(image,sizeof(dpxinfo.file.copyright),(unsigned char *)
    dpxinfo.file.copyright);
  if (*dpxinfo.file.copyright != '\0')
    {
      (void) FormatImageProperty(image,"dpx:file.copyright","%.200s",
        dpxinfo.file.copyright);
      (void) FormatImageProperty(image,"copyright","%.100s",
        dpxinfo.file.copyright);
    }
  dpxinfo.file.encrypt_key=ReadBlobLong(image);
  offset+=4;
  if (dpxinfo.file.encrypt_key != ~0U)
    (void) FormatImageProperty(image,"dpx:file.encrypt_key","%u",
      dpxinfo.file.encrypt_key);
  offset+=ReadBlob(image,sizeof(dpxinfo.file.reserve),(unsigned char *)
    dpxinfo.file.reserve);
  /*
    Read DPX image header.
  */
  dpxinfo.image.orientation=ReadBlobShort(image);
  if (dpxinfo.image.orientation > 7)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  offset+=2;
  if (dpxinfo.image.orientation != (unsigned short) ~0)
    (void) FormatImageProperty(image,"dpx:image.orientation","%d",
      dpxinfo.image.orientation);
  switch (dpxinfo.image.orientation)
  {
    default:
    case 0: image->orientation=TopLeftOrientation; break;
    case 1: image->orientation=TopRightOrientation; break;
    case 2: image->orientation=BottomLeftOrientation; break;
    case 3: image->orientation=BottomRightOrientation; break;
    case 4: image->orientation=LeftTopOrientation; break;
    case 5: image->orientation=RightTopOrientation; break;
    case 6: image->orientation=LeftBottomOrientation; break;
    case 7: image->orientation=RightBottomOrientation; break;
  }
  dpxinfo.image.number_elements=ReadBlobShort(image);
  if ((dpxinfo.image.number_elements < 1) ||
      (dpxinfo.image.number_elements > MaxNumberImageElements))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  offset+=2;
  dpxinfo.image.pixels_per_line=ReadBlobLong(image);
  offset+=4;
  image->columns=dpxinfo.image.pixels_per_line;
  dpxinfo.image.lines_per_element=ReadBlobLong(image);
  offset+=4;
  image->rows=dpxinfo.image.lines_per_element;
  for (i=0; i < 8; i++)
  {
    char
      property[MagickPathExtent];

    dpxinfo.image.image_element[i].data_sign=ReadBlobLong(image);
    offset+=4;
    dpxinfo.image.image_element[i].low_data=ReadBlobLong(image);
    offset+=4;
    dpxinfo.image.image_element[i].low_quantity=ReadBlobFloat(image);
    offset+=4;
    dpxinfo.image.image_element[i].high_data=ReadBlobLong(image);
    offset+=4;
    dpxinfo.image.image_element[i].high_quantity=ReadBlobFloat(image);
    offset+=4;
    dpxinfo.image.image_element[i].descriptor=(unsigned char) ReadBlobByte(image);
    offset++;
    dpxinfo.image.image_element[i].transfer_characteristic=(unsigned char)
      ReadBlobByte(image);
    (void) FormatLocaleString(property,MagickPathExtent,
      "dpx:image.element[%lu].transfer-characteristic",(long) i);
    (void) FormatImageProperty(image,property,"%s",
      GetImageTransferCharacteristic((DPXTransferCharacteristic)
      dpxinfo.image.image_element[i].transfer_characteristic));
    offset++;
    dpxinfo.image.image_element[i].colorimetric=(unsigned char) ReadBlobByte(image);
    offset++;
    dpxinfo.image.image_element[i].bit_size=(unsigned char) ReadBlobByte(image);
    offset++;
    dpxinfo.image.image_element[i].packing=ReadBlobShort(image);
    offset+=2;
    dpxinfo.image.image_element[i].encoding=ReadBlobShort(image);
    offset+=2;
    dpxinfo.image.image_element[i].data_offset=ReadBlobLong(image);
    offset+=4;
    dpxinfo.image.image_element[i].end_of_line_padding=ReadBlobLong(image);
    offset+=4;
    dpxinfo.image.image_element[i].end_of_image_padding=ReadBlobLong(image);
    offset+=4;
    offset+=ReadBlob(image,sizeof(dpxinfo.image.image_element[i].description),
      (unsigned char *) dpxinfo.image.image_element[i].description);
  }
  SetImageColorspace(image,RGBColorspace,exception);
  offset+=ReadBlob(image,sizeof(dpxinfo.image.reserve),(unsigned char *)
    dpxinfo.image.reserve);
  if (dpxinfo.file.image_offset >= 1664U)
    {
      /*
        Read DPX orientation header.
      */
      dpxinfo.orientation.x_offset=ReadBlobLong(image);
      offset+=4;
      if (dpxinfo.orientation.x_offset != ~0U)
        (void) FormatImageProperty(image,"dpx:orientation.x_offset","%u",
          dpxinfo.orientation.x_offset);
      dpxinfo.orientation.y_offset=ReadBlobLong(image);
      offset+=4;
      if (dpxinfo.orientation.y_offset != ~0U)
        (void) FormatImageProperty(image,"dpx:orientation.y_offset","%u",
          dpxinfo.orientation.y_offset);
      dpxinfo.orientation.x_center=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpxinfo.orientation.x_center) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:orientation.x_center","%g",
          dpxinfo.orientation.x_center);
      dpxinfo.orientation.y_center=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpxinfo.orientation.y_center) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:orientation.y_center","%g",
          dpxinfo.orientation.y_center);
      dpxinfo.orientation.x_size=ReadBlobLong(image);
      offset+=4;
      if (dpxinfo.orientation.x_size != ~0U)
        (void) FormatImageProperty(image,"dpx:orientation.x_size","%u",
          dpxinfo.orientation.x_size);
      dpxinfo.orientation.y_size=ReadBlobLong(image);
      offset+=4;
      if (dpxinfo.orientation.y_size != ~0U)
        (void) FormatImageProperty(image,"dpx:orientation.y_size","%u",
          dpxinfo.orientation.y_size);
      offset+=ReadBlob(image,sizeof(dpxinfo.orientation.filename),(unsigned char *)
        dpxinfo.orientation.filename);
      if (*dpxinfo.orientation.filename != '\0')
        (void) FormatImageProperty(image,"dpx:orientation.filename","%.100s",
          dpxinfo.orientation.filename);
      offset+=ReadBlob(image,sizeof(dpxinfo.orientation.timestamp),(unsigned char *)
        dpxinfo.orientation.timestamp);
      if (*dpxinfo.orientation.timestamp != '\0')
        (void) FormatImageProperty(image,"dpx:orientation.timestamp","%.24s",
          dpxinfo.orientation.timestamp);
      offset+=ReadBlob(image,sizeof(dpxinfo.orientation.device),(unsigned char *)
        dpxinfo.orientation.device);
      if (*dpxinfo.orientation.device != '\0')
        (void) FormatImageProperty(image,"dpx:orientation.device","%.32s",
          dpxinfo.orientation.device);
      offset+=ReadBlob(image,sizeof(dpxinfo.orientation.serial),(unsigned char *)
        dpxinfo.orientation.serial);
      if (*dpxinfo.orientation.serial != '\0')
        (void) FormatImageProperty(image,"dpx:orientation.serial","%.32s",
          dpxinfo.orientation.serial);
      for (i=0; i < 4; i++)
      {
        dpxinfo.orientation.border[i]=ReadBlobShort(image);
        offset+=2;
      }
      if ((dpxinfo.orientation.border[0] != (unsigned short) (~0)) &&
          (dpxinfo.orientation.border[1] != (unsigned short) (~0)))
        (void) FormatImageProperty(image,"dpx:orientation.border","%dx%d%+d%+d",
          dpxinfo.orientation.border[0],dpxinfo.orientation.border[1],
          dpxinfo.orientation.border[2],dpxinfo.orientation.border[3]);
      for (i=0; i < 2; i++)
      {
        dpxinfo.orientation.aspect_ratio[i]=ReadBlobLong(image);
        offset+=4;
      }
      if ((dpxinfo.orientation.aspect_ratio[0] != ~0U) &&
          (dpxinfo.orientation.aspect_ratio[1] != ~0U))
        (void) FormatImageProperty(image,"dpx:orientation.aspect_ratio",
          "%ux%u",dpxinfo.orientation.aspect_ratio[0],
          dpxinfo.orientation.aspect_ratio[1]);
      offset+=ReadBlob(image,sizeof(dpxinfo.orientation.reserve),(unsigned char *)
        dpxinfo.orientation.reserve);
    }
  if (dpxinfo.file.image_offset >= 1920U)
    {
      /*
        Read DPX film header.
      */
      offset+=ReadBlob(image,sizeof(dpxinfo.film.id),(unsigned char *) dpxinfo.film.id);
      if (*dpxinfo.film.id != '\0')
        (void) FormatImageProperty(image,"dpx:film.id","%.2s",dpxinfo.film.id);
      offset+=ReadBlob(image,sizeof(dpxinfo.film.type),(unsigned char *)
        dpxinfo.film.type);
      if (*dpxinfo.film.type != '\0')
        (void) FormatImageProperty(image,"dpx:film.type","%.2s",dpxinfo.film.type);
      offset+=ReadBlob(image,sizeof(dpxinfo.film.offset),(unsigned char *)
        dpxinfo.film.offset);
      if (*dpxinfo.film.offset != '\0')
        (void) FormatImageProperty(image,"dpx:film.offset","%.2s",
          dpxinfo.film.offset);
      offset+=ReadBlob(image,sizeof(dpxinfo.film.prefix),(unsigned char *)
        dpxinfo.film.prefix);
      if (*dpxinfo.film.prefix != '\0')
        (void) FormatImageProperty(image,"dpx:film.prefix","%.6s",
          dpxinfo.film.prefix);
      offset+=ReadBlob(image,sizeof(dpxinfo.film.count),(unsigned char *)
        dpxinfo.film.count);
      if (*dpxinfo.film.count != '\0')
        (void) FormatImageProperty(image,"dpx:film.count","%.4s",
          dpxinfo.film.count);
      offset+=ReadBlob(image,sizeof(dpxinfo.film.format),(unsigned char *)
        dpxinfo.film.format);
      if (*dpxinfo.film.format != '\0')
        (void) FormatImageProperty(image,"dpx:film.format","%.4s",
          dpxinfo.film.format);
      dpxinfo.film.frame_position=ReadBlobLong(image);
      offset+=4;
      if (dpxinfo.film.frame_position != ~0U)
        (void) FormatImageProperty(image,"dpx:film.frame_position","%u",
          dpxinfo.film.frame_position);
      dpxinfo.film.sequence_extent=ReadBlobLong(image);
      offset+=4;
      if (dpxinfo.film.sequence_extent != ~0U)
        (void) FormatImageProperty(image,"dpx:film.sequence_extent","%u",
          dpxinfo.film.sequence_extent);
      dpxinfo.film.held_count=ReadBlobLong(image);
      offset+=4;
      if (dpxinfo.film.held_count != ~0U)
        (void) FormatImageProperty(image,"dpx:film.held_count","%u",
          dpxinfo.film.held_count);
      dpxinfo.film.frame_rate=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpxinfo.film.frame_rate) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:film.frame_rate","%g",
          dpxinfo.film.frame_rate);
      dpxinfo.film.shutter_angle=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpxinfo.film.shutter_angle) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:film.shutter_angle","%g",
          dpxinfo.film.shutter_angle);
      offset+=ReadBlob(image,sizeof(dpxinfo.film.frame_id),(unsigned char *)
        dpxinfo.film.frame_id);
      if (*dpxinfo.film.frame_id != '\0')
        (void) FormatImageProperty(image,"dpx:film.frame_id","%.32s",
          dpxinfo.film.frame_id);
      offset+=ReadBlob(image,sizeof(dpxinfo.film.slate),(unsigned char *)
        dpxinfo.film.slate);
      if (*dpxinfo.film.slate != '\0')
        (void) FormatImageProperty(image,"dpx:film.slate","%.100s",
          dpxinfo.film.slate);
      offset+=ReadBlob(image,sizeof(dpxinfo.film.reserve),(unsigned char *)
        dpxinfo.film.reserve);
    }
  if (dpxinfo.file.image_offset >= 2048U)
    {
      /*
        Read DPX television header.
      */
      dpxinfo.television.time_code=(unsigned int) ReadBlobLong(image);
      offset+=4;
      TimeCodeToString(dpxinfo.television.time_code,value);
      (void) SetImageProperty(image,"dpx:television.time.code",value,exception);
      dpxinfo.television.user_bits=(unsigned int) ReadBlobLong(image);
      offset+=4;
      TimeCodeToString(dpxinfo.television.user_bits,value);
      (void) SetImageProperty(image,"dpx:television.user.bits",value,exception);
      dpxinfo.television.interlace=(unsigned char) ReadBlobByte(image);
      offset++;
      if (dpxinfo.television.interlace != 0)
        (void) FormatImageProperty(image,"dpx:television.interlace","%.20g",
          (double) dpxinfo.television.interlace);
      dpxinfo.television.field_number=(unsigned char) ReadBlobByte(image);
      offset++;
      if (dpxinfo.television.field_number != 0)
        (void) FormatImageProperty(image,"dpx:television.field_number","%.20g",
          (double) dpxinfo.television.field_number);
      dpxinfo.television.video_signal=(unsigned char) ReadBlobByte(image);
      offset++;
      if (dpxinfo.television.video_signal != 0)
        (void) FormatImageProperty(image,"dpx:television.video_signal","%.20g",
          (double) dpxinfo.television.video_signal);
      dpxinfo.television.padding=(unsigned char) ReadBlobByte(image);
      offset++;
      if (dpxinfo.television.padding != 0)
        (void) FormatImageProperty(image,"dpx:television.padding","%d",
          dpxinfo.television.padding);
      dpxinfo.television.horizontal_sample_rate=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpxinfo.television.horizontal_sample_rate) != MagickFalse)
        (void) FormatImageProperty(image,
          "dpx:television.horizontal_sample_rate","%g",
          dpxinfo.television.horizontal_sample_rate);
      dpxinfo.television.vertical_sample_rate=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpxinfo.television.vertical_sample_rate) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:television.vertical_sample_rate",
          "%g",dpxinfo.television.vertical_sample_rate);
      dpxinfo.television.frame_rate=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpxinfo.television.frame_rate) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:television.frame_rate","%g",
          dpxinfo.television.frame_rate);
      dpxinfo.television.time_offset=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpxinfo.television.time_offset) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:television.time_offset","%g",
          dpxinfo.television.time_offset);
      dpxinfo.television.gamma=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpxinfo.television.gamma) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:television.gamma","%g",
          dpxinfo.television.gamma);
      dpxinfo.television.black_level=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpxinfo.television.black_level) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:television.black_level","%g",
          dpxinfo.television.black_level);
      dpxinfo.television.black_gain=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpxinfo.television.black_gain) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:television.black_gain","%g",
          dpxinfo.television.black_gain);
      dpxinfo.television.break_point=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpxinfo.television.break_point) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:television.break_point","%g",
          dpxinfo.television.break_point);
      dpxinfo.television.white_level=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpxinfo.television.white_level) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:television.white_level","%g",
          dpxinfo.television.white_level);
      dpxinfo.television.integration_times=ReadBlobFloat(image);
      offset+=4;
      if (IsFloatDefined(dpxinfo.television.integration_times) != MagickFalse)
        (void) FormatImageProperty(image,"dpx:television.integration_times",
          "%g",dpxinfo.television.integration_times);
      offset+=ReadBlob(image,sizeof(dpxinfo.television.reserve),(unsigned char *)
        dpxinfo.television.reserve);
    }
  if (dpxinfo.file.image_offset > 2080U)
    {
      /*
        Read DPX user header.
      */
      offset+=ReadBlob(image,sizeof(dpxinfo.user.id),(unsigned char *) dpxinfo.user.id);
      if (*dpxinfo.user.id != '\0')
        (void) FormatImageProperty(image,"dpx:user.id","%.32s",dpxinfo.user.id);
      if ((dpxinfo.file.user_size != ~0U) &&
          ((size_t) dpxinfo.file.user_size > sizeof(dpxinfo.user.id)))
        {
          StringInfo
            *profile;

           if (dpxinfo.file.user_size > GetBlobSize(image))
             ThrowReaderException(CorruptImageError,
               "InsufficientImageDataInFile");
           profile=BlobToStringInfo((const unsigned char *) NULL,
             dpxinfo.file.user_size-sizeof(dpxinfo.user.id));
           if (profile == (StringInfo *) NULL)
             ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
           offset+=ReadBlob(image,GetStringInfoLength(profile),
             GetStringInfoDatum(profile));
           (void) SetImageProfile(image,"dpx:user-data",profile,exception);
           profile=DestroyStringInfo(profile);
        }
    }
  for ( ; offset < (MagickOffsetType) dpxinfo.file.image_offset; offset++)
    if (ReadBlobByte(image) == EOF)
      break;
  if (EOFBlob(image) != MagickFalse)
    ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  if (image_info->ping != MagickFalse)
    {
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  (void) SetImageBackgroundColor(image,exception);
  for (n=0; n < (ssize_t) dpxinfo.image.number_elements; n++)
  {
    /*
      Convert DPX raster image to pixel packets.
    */
    if ((dpxinfo.image.image_element[n].data_offset != ~0U) &&
        (dpxinfo.image.image_element[n].data_offset != 0U))
      {
         MagickOffsetType
           data_offset;

         data_offset=(MagickOffsetType) dpxinfo.image.image_element[n].data_offset;
         if (data_offset < offset)
           offset=SeekBlob(image,data_offset,SEEK_SET);
         else
           for ( ; offset < data_offset; offset++)
             if (ReadBlobByte(image) == EOF)
               break;
          if (offset != data_offset)
            ThrowReaderException(CorruptImageError,"UnableToReadImageData");
       }
    SetPrimaryChromaticity((DPXColorimetric)
      dpxinfo.image.image_element[n].colorimetric,&image->chromaticity);
    image->depth=dpxinfo.image.image_element[n].bit_size;
    if ((image->depth == 0) || (image->depth > 32))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    samples_per_pixel=1;
    quantum_type=GrayQuantum;
    component_type=dpxinfo.image.image_element[n].descriptor;
    switch (component_type)
    {
      case CbYCrY422ComponentType:
      {
        samples_per_pixel=2;
        quantum_type=CbYCrYQuantum;
        break;
      }
      case CbYACrYA4224ComponentType:
      case CbYCr444ComponentType:
      {
        samples_per_pixel=3;
        quantum_type=CbYCrQuantum;
        break;
      }
      case RGBComponentType:
      {
        samples_per_pixel=3;
        quantum_type=RGBQuantum;
        break;
      }
      case ABGRComponentType:
      case RGBAComponentType:
      {
        image->alpha_trait=BlendPixelTrait;
        samples_per_pixel=4;
        quantum_type=RGBAQuantum;
        break;
      }
      default:
        break;
    }
    switch (component_type)
    {
      case CbYCrY422ComponentType:
      case CbYACrYA4224ComponentType:
      case CbYCr444ComponentType:
      {
        SetImageColorspace(image,Rec709YCbCrColorspace,exception);
        break;
      }
      case LumaComponentType:
      {
        SetImageColorspace(image,GRAYColorspace,exception);
        break;
      }
      default:
      {
        SetImageColorspace(image,sRGBColorspace,exception);
        if (dpxinfo.image.image_element[n].transfer_characteristic == LogarithmicColorimetric)
          SetImageColorspace(image,LogColorspace,exception);
        if (dpxinfo.image.image_element[n].transfer_characteristic == PrintingDensityColorimetric)
          SetImageColorspace(image,LogColorspace,exception);
        break;
      }
    }
    extent=GetBytesPerRow(image->columns,samples_per_pixel,image->depth,
      dpxinfo.image.image_element[n].packing == 0 ? MagickFalse : MagickTrue);
    /*
      DPX any-bit pixel format.
    */
    row=0;
    quantum_info=AcquireQuantumInfo(image_info,image);
    if (quantum_info == (QuantumInfo *) NULL)
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    SetQuantumQuantum(quantum_info,32);
    SetQuantumPack(quantum_info,dpxinfo.image.image_element[n].packing == 0 ?
      MagickTrue : MagickFalse);
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      const unsigned char
        *pixels;

      MagickBooleanType
        sync;

      register Quantum
        *q;

      size_t
        length;

      ssize_t
        offset;

      pixels=(const unsigned char *) ReadBlobStream(image,extent,
        GetQuantumPixels(quantum_info),&count);
      if (count != (ssize_t) extent)
        break;
      if ((image->progress_monitor != (MagickProgressMonitor) NULL) &&
          (image->previous == (Image *) NULL))
        {
          MagickBooleanType
            proceed;

          proceed=SetImageProgress(image,LoadImageTag,(MagickOffsetType) row,
            image->rows);
          if (proceed == MagickFalse)
            break;
        }
      offset=row++;
      q=QueueAuthenticPixels(image,0,offset,image->columns,1,exception);
      if (q == (Quantum *) NULL)
        break;
      length=ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
        quantum_type,pixels,exception);
      (void) length;
      sync=SyncAuthenticPixels(image,exception);
      if (sync == MagickFalse)
        break;
    }
    quantum_info=DestroyQuantumInfo(quantum_info);
    if (y < (ssize_t) image->rows)
      ThrowReaderException(CorruptImageError,"UnableToReadImageData");
    SetQuantumImageType(image,quantum_type);
    if (EOFBlob(image) != MagickFalse)
      ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
        image->filename);
    if ((i+1) < (ssize_t) dpxinfo.image.number_elements)
      {
        /*
          Allocate next image structure.
        */
        AcquireNextImage(image_info,image,exception);
        if (GetNextImageInList(image) == (Image *) NULL)
          {
            image=DestroyImageList(image);
            return((Image *) NULL);
          }
        image=SyncNextImageInList(image);
        status=SetImageProgress(image,LoadImagesTag,TellBlob(image),
          GetBlobSize(image));
        if (status == MagickFalse)
          break;
      }
  }
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}