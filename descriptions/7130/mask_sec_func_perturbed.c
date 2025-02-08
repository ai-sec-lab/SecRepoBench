static Image *ReadPICTImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
#define ThrowPICTException(exception,message) \
{ \
  if (tile_image != (Image *) NULL) \
    tile_image=DestroyImage(tile_image); \
  if (read_info != (ImageInfo *) NULL) \
    read_info=DestroyImageInfo(read_info); \
  ThrowReaderException((exception),(message)); \
}

  char
    geometry[MagickPathExtent],
    header_ole[4];

  Image
    *image,
    *tile_image;

  ImageInfo
    *read_info;

  int
    c,
    code;

  MagickBooleanType
    jpeg,
    status;

  PICTRectangle
    frame;

  PICTPixmap
    pixmap;

  Quantum
    index;

  register Quantum
    *q;

  register ssize_t
    i,
    x;

  size_t
    extent,
    length;

  ssize_t
    count,
    flags,
    j,
    version,
    y;

  StringInfo
    *profile;

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
    Read PICT header.
  */
  read_info=(ImageInfo *) NULL;
  tile_image=(Image *) NULL;
  pixmap.bits_per_pixel=0;
  pixmap.component_count=0;
  /*
    Skip header : 512 for standard PICT and 4, ie "PICT" for OLE2.
  */
  header_ole[0]=ReadBlobByte(image);
  header_ole[1]=ReadBlobByte(image);
  header_ole[2]=ReadBlobByte(image);
  header_ole[3]=ReadBlobByte(image);
  if (!((header_ole[0] == 0x50) && (header_ole[1] == 0x49) &&
      (header_ole[2] == 0x43) && (header_ole[3] == 0x54 )))
    for (i=0; i < 508; i++)
      if (ReadBlobByte(image) == EOF)
        break;
  (void) ReadBlobMSBShort(image);  /* skip picture size */
  if (ReadRectangle(image,&frame) == MagickFalse)
    ThrowPICTException(CorruptImageError,"ImproperImageHeader");
  while ((c=ReadBlobByte(image)) == 0) ;
  if (c != 0x11)
    ThrowPICTException(CorruptImageError,"ImproperImageHeader");
  version=(ssize_t) ReadBlobByte(image);
  if (version == 2)
    {
      c=ReadBlobByte(image);
      if (c != 0xff)
        ThrowPICTException(CorruptImageError,"ImproperImageHeader");
    }
  else
    if (version != 1)
      ThrowPICTException(CorruptImageError,"ImproperImageHeader");
  if ((frame.left < 0) || (frame.right < 0) || (frame.top < 0) ||
      (frame.bottom < 0) || (frame.left >= frame.right) ||
      (frame.top >= frame.bottom))
    ThrowPICTException(CorruptImageError,"ImproperImageHeader");
  /*
    Create black canvas.
  */
  flags=0;
  image->depth=8;
  image->columns=(size_t) (frame.right-frame.left);
  image->rows=(size_t) (frame.bottom-frame.top);
  image->resolution.x=DefaultResolution;
  image->resolution.y=DefaultResolution;
  image->units=UndefinedResolution;
  if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
    if (image->scene >= (image_info->scene+image_info->number_scenes-1))
      {
        (void) CloseBlob(image);
        return(GetFirstImageInList(image));
      }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  status=ResetImagePixels(image,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  /*
    Interpret PICT opcodes.
  */
  jpeg=MagickFalse;
  for (code=0; EOFBlob(image) == MagickFalse; )
  {
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    if ((version == 1) || ((TellBlob(image) % 2) != 0))
      code=ReadBlobByte(image);
    if (version == 2)
      code=ReadBlobMSBSignedShort(image);
    if (code < 0)
      break;
    if (code == 0)
      continue;
    if (code > 0xa1)
      {
        if (image->debug != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),"%04X:",code);
      }
    else
      {
        if (image->debug != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "  %04X %s: %s",code,codes[code].name,codes[code].description);
        switch (code)
        {
          case 0x01:
          {
            /*
              Clipping rectangle.
            */
            length=ReadBlobMSBShort(image);
            if (length != 0x000a)
              {
                for (i=0; i < (ssize_t) (length-2); i++)
                  if (ReadBlobByte(image) == EOF)
                    break;
                break;
              }
            if (ReadRectangle(image,&frame) == MagickFalse)
              ThrowPICTException(CorruptImageError,"ImproperImageHeader");
            if (((frame.left & 0x8000) != 0) || ((frame.top & 0x8000) != 0))
              break;
            image->columns=(size_t) (frame.right-frame.left);
            image->rows=(size_t) (frame.bottom-frame.top);
            status=SetImageExtent(image,image->columns,image->rows,exception);
            if (status == MagickFalse)
              return(DestroyImageList(image));
            status=ResetImagePixels(image,exception);
            if (status == MagickFalse)
              return(DestroyImageList(image));
            break;
          }
          case 0x12:
          case 0x13:
          case 0x14:
          {
            ssize_t
              pattern;

            size_t
              height,
              width;

            /*
              Skip pattern definition.
            */
            pattern=(ssize_t) ReadBlobMSBShort(image);
            for (i=0; i < 8; i++)
              if (ReadBlobByte(image) == EOF)
                break;
            if (pattern == 2)
              {
                for (i=0; i < 5; i++)
                  if (ReadBlobByte(image) == EOF)
                    break;
                break;
              }
            if (pattern != 1)
              ThrowPICTException(CorruptImageError,"UnknownPatternType");
            length=ReadBlobMSBShort(image);
            if (ReadRectangle(image,&frame) == MagickFalse)
              ThrowPICTException(CorruptImageError,"ImproperImageHeader");
            if (ReadPixmap(image,&pixmap) == MagickFalse)
              ThrowPICTException(CorruptImageError,"ImproperImageHeader");
            image->depth=(size_t) pixmap.component_size;
            image->resolution.x=1.0*pixmap.horizontal_resolution;
            image->resolution.y=1.0*pixmap.vertical_resolution;
            image->units=PixelsPerInchResolution;
            (void) ReadBlobMSBLong(image);
            flags=(ssize_t) ReadBlobMSBShort(image);
            length=ReadBlobMSBShort(image);
            for (i=0; i <= (ssize_t) length; i++)
              (void) ReadBlobMSBLong(image);
            width=(size_t) (frame.bottom-frame.top);
            height=(size_t) (frame.right-frame.left);
            if (pixmap.bits_per_pixel <= 8)
              length&=0x7fff;
            if (pixmap.bits_per_pixel == 16)
              width<<=1;
            if (length == 0)
              length=width;
            if (length < 8)
              {
                for (i=0; i < (ssize_t) (length*height); i++)
                  if (ReadBlobByte(image) == EOF)
                    break;
              }
            else
              for (i=0; i < (ssize_t) height; i++)
              {
                if (EOFBlob(image) != MagickFalse)
                  break;
                if (length > 200)
                  {
                    for (j=0; j < (ssize_t) ReadBlobMSBShort(image); j++)
                      if (ReadBlobByte(image) == EOF)
                        break;
                  }
                else
                  for (j=0; j < (ssize_t) ReadBlobByte(image); j++)
                    if (ReadBlobByte(image) == EOF)
                      break;
              }
            break;
          }
          case 0x1b:
          {
            /*
              Initialize image background color.
            */
            image->background_color.red=(Quantum)
              ScaleShortToQuantum(ReadBlobMSBShort(image));
            image->background_color.green=(Quantum)
              ScaleShortToQuantum(ReadBlobMSBShort(image));
            image->background_color.blue=(Quantum)
              ScaleShortToQuantum(ReadBlobMSBShort(image));
            break;
          }
          case 0x70:
          case 0x71:
          case 0x72:
          case 0x73:
          case 0x74:
          case 0x75:
          case 0x76:
          case 0x77:
          {
            /*
              Skip polygon or region.
            */
            length=ReadBlobMSBShort(image);
            for (i=0; i < (ssize_t) (length-2); i++)
              if (ReadBlobByte(image) == EOF)
                break;
            break;
          }
          case 0x90:
          case 0x91:
          case 0x98:
          case 0x99:
          case 0x9a:
          case 0x9b:
          {
            PICTRectangle
              source,
              destination;

            register unsigned char
              *p;

            size_t
              j;

            ssize_t
              bytesperrow;

            unsigned char
              *pixels;

            /*
              Pixmap clipped by a rectangle.
            */
            bytesperrow=0;
            if ((code != 0x9a) && (code != 0x9b))
              bytesperrow=(ssize_t) ReadBlobMSBShort(image);
            else
              {
                (void) ReadBlobMSBShort(image);
                (void) ReadBlobMSBShort(image);
                (void) ReadBlobMSBShort(image);
              }
            if (ReadRectangle(image,&frame) == MagickFalse)
              ThrowPICTException(CorruptImageError,"ImproperImageHeader");
            /*
              Initialize tile image.
            */
            tile_image=CloneImage(image,(size_t) (frame.right-frame.left),
              (size_t) (frame.bottom-frame.top),MagickTrue,exception);
            if (tile_image == (Image *) NULL)
              ThrowPICTException(CorruptImageError,"ImproperImageHeader");
            // <MASK>
          }
          case 0xa1:
          {
            unsigned char
              *info;

            size_t
              type;

            /*
              Comment.
            */
            type=ReadBlobMSBShort(image);
            length=ReadBlobMSBShort(image);
            if (length == 0)
              break;
            (void) ReadBlobMSBLong(image);
            length-=MagickMin(length,4);
            if (length == 0)
              break;
            info=(unsigned char *) AcquireQuantumMemory(length,sizeof(*info));
            if (info == (unsigned char *) NULL)
              break;
            count=ReadBlob(image,length,info);
            if (count != (ssize_t) length)
              {
                info=(unsigned char *) RelinquishMagickMemory(info);
                ThrowPICTException(ResourceLimitError,"UnableToReadImageData");
              }
            switch (type)
            {
              case 0xe0:
              {
                profile=BlobToStringInfo((const void *) NULL,length);
                SetStringInfoDatum(profile,info);
                status=SetImageProfile(image,"icc",profile,exception);
                profile=DestroyStringInfo(profile);
                if (status == MagickFalse)
                  {
                    info=(unsigned char *) RelinquishMagickMemory(info);
                    ThrowPICTException(ResourceLimitError,
                      "MemoryAllocationFailed");
                  }
                break;
              }
              case 0x1f2:
              {
                profile=BlobToStringInfo((const void *) NULL,length);
                SetStringInfoDatum(profile,info);
                status=SetImageProfile(image,"iptc",profile,exception);
                if (status == MagickFalse)
                  {
                    info=(unsigned char *) RelinquishMagickMemory(info);
                    ThrowPICTException(ResourceLimitError,
                      "MemoryAllocationFailed");
                  }
                profile=DestroyStringInfo(profile);
                break;
              }
              default:
                break;
            }
            info=(unsigned char *) RelinquishMagickMemory(info);
            break;
          }
          default:
          {
            /*
              Skip to next op code.
            */
            if (codes[code].length == -1)
              (void) ReadBlobMSBShort(image);
            else
              for (i=0; i < (ssize_t) codes[code].length; i++)
                if (ReadBlobByte(image) == EOF)
                  break;
          }
        }
      }
    if (code == 0xc00)
      {
        /*
          Skip header.
        */
        for (i=0; i < 24; i++)
          if (ReadBlobByte(image) == EOF)
            break;
        continue;
      }
    if (((code >= 0xb0) && (code <= 0xcf)) ||
        ((code >= 0x8000) && (code <= 0x80ff)))
      continue;
    if (code == 0x8200)
      {
        char
          filename[MaxTextExtent];

        FILE
          *file;

        int
          unique_file;

        /*
          Embedded JPEG.
        */
        jpeg=MagickTrue;
        read_info=CloneImageInfo(image_info);
        SetImageInfoBlob(read_info,(void *) NULL,0);
        file=(FILE *) NULL;
        unique_file=AcquireUniqueFileResource(filename);
        (void) FormatLocaleString(read_info->filename,MaxTextExtent,"jpeg:%s",
          filename);
        if (unique_file != -1)
          file=fdopen(unique_file,"wb");
        if ((unique_file == -1) || (file == (FILE *) NULL))
          {
            (void) RelinquishUniqueFileResource(read_info->filename);
            (void) CopyMagickString(image->filename,read_info->filename,
              MagickPathExtent);
            ThrowFileException(exception,FileOpenError,
              "UnableToCreateTemporaryFile",image->filename);
            image=DestroyImageList(image);
            return((Image *) NULL);
          }
        length=ReadBlobMSBLong(image);
        if (length > 154)
          {
            for (i=0; i < 6; i++)
              (void) ReadBlobMSBLong(image);
            if (ReadRectangle(image,&frame) == MagickFalse)
              {
                (void) fclose(file);
                (void) RelinquishUniqueFileResource(read_info->filename);
                ThrowPICTException(CorruptImageError,"ImproperImageHeader");
              }
            for (i=0; i < 122; i++)
              if (ReadBlobByte(image) == EOF)
                break;
            for (i=0; i < (ssize_t) (length-154); i++)
            {
              c=ReadBlobByte(image);
              if (c == EOF)
                break;
              (void) fputc(c,file);
            }
          }
        (void) fclose(file);
        (void) close(unique_file);
        tile_image=ReadImage(read_info,exception);
        (void) RelinquishUniqueFileResource(filename);
        read_info=DestroyImageInfo(read_info);
        if (tile_image == (Image *) NULL)
          continue;
        (void) FormatLocaleString(geometry,MagickPathExtent,"%.20gx%.20g",
          (double) MagickMax(image->columns,tile_image->columns),
          (double) MagickMax(image->rows,tile_image->rows));
        (void) SetImageExtent(image,
          MagickMax(image->columns,tile_image->columns),
          MagickMax(image->rows,tile_image->rows),exception);
        (void) TransformImageColorspace(image,tile_image->colorspace,exception);
        (void) CompositeImage(image,tile_image,CopyCompositeOp,MagickTrue,
          (ssize_t) frame.left,(ssize_t) frame.right,exception);
        image->compression=tile_image->compression;
        tile_image=DestroyImage(tile_image);
        continue;
      }
    if ((code == 0xff) || (code == 0xffff))
      break;
    if (((code >= 0xd0) && (code <= 0xfe)) ||
        ((code >= 0x8100) && (code <= 0xffff)))
      {
        /*
          Skip reserved.
        */
        length=ReadBlobMSBShort(image);
        for (i=0; i < (ssize_t) length; i++)
          if (ReadBlobByte(image) == EOF)
            break;
        continue;
      }
    if ((code >= 0x100) && (code <= 0x7fff))
      {
        /*
          Skip reserved.
        */
        length=(size_t) ((code >> 7) & 0xff);
        for (i=0; i < (ssize_t) length; i++)
          if (ReadBlobByte(image) == EOF)
            break;
        continue;
      }
  }
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}