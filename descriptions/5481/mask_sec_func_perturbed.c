static Image *ReadPESImage(const ImageInfo *image_info,ExceptionInfo *errorinfo)
{
  char
    filename[MagickPathExtent];

  FILE
    *file;

  Image
    *image;

  ImageInfo
    *read_info;

  int
    delta_x,
    delta_y,
    j,
    unique_file,
    x,
    y;

  MagickBooleanType
    status;

  PESBlockInfo
    blocks[256];

  PointInfo
    *stitches;

  SegmentInfo
    bounds;

  register ssize_t
    i;

  size_t
    number_blocks,
    number_colors,
    number_stitches;

  ssize_t
    count,
    offset;

  unsigned char
    magick[4],
    version[4];

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(errorinfo != (ExceptionInfo *) NULL);
  assert(errorinfo->signature == MagickCoreSignature);
  image=AcquireImage(image_info,errorinfo);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,errorinfo);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Verify PES identifier.
  */
  count=ReadBlob(image,4,magick);
  if ((count != 4) || (LocaleNCompare((char *) magick,"#PES",4) != 0))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  count=ReadBlob(image,4,version);
  offset=ReadBlobLSBSignedLong(image);
  if (DiscardBlobBytes(image,offset+36) == MagickFalse)
    ThrowFileException(errorinfo,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  if (EOFBlob(image) != MagickFalse)
    ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
  /*
    Get PES colors.
  */
  number_colors=(size_t) ReadBlobByte(image)+1;
  for (i=0; i < (ssize_t) number_colors; i++)
  {
    j=ReadBlobByte(image);
    blocks[i].color=PESColor+(j < 0 ? 0 : j);
    blocks[i].offset=0;
  }
  for ( ; i < 256L; i++)
  {
    blocks[i].offset=0;
    blocks[i].color=PESColor;
  }
  if (DiscardBlobBytes(image,532L-number_colors-21) == MagickFalse)
    ThrowFileException(errorinfo,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  if (EOFBlob(image) != MagickFalse)
    ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
  /*
    Stitch away.
  */
  number_stitches=64;
  stitches=(PointInfo *) AcquireQuantumMemory(number_stitches,
    sizeof(*stitches));
  if (stitches == (PointInfo *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  bounds.x1=65535.0;
  bounds.y1=65535.0;
  bounds.x2=(-65535.0);
  bounds.y2=(-65535.0);
  i=0;
  j=0;
  delta_x=0;
  delta_y=0;
  while (EOFBlob(image) == MagickFalse)
  {
    x=ReadBlobByte(image);
    y=ReadBlobByte(image);
    if ((x == 0xff) && (y == 0))
      break;
    if ((x == 254) && (y == 176))
      {
        // <MASK>
        (void) ReadBlobByte(image);
        continue;
      }
    if ((x & 0x80) == 0)
      {
        /*
          Normal stitch.
        */
        if ((x & 0x40) != 0)
          x-=0x80;
      }
    else
      {
        /*
          Jump stitch.
        */
        x=((x & 0x0f) << 8)+y;
        if ((x & 0x800) != 0)
          x-=0x1000;
        y=ReadBlobByte(image);
      }
    if ((y & 0x80) == 0)
      {
        /*
          Normal stitch.
        */
        if ((y & 0x40) != 0)
          y-=0x80;
      }
    else
      {
        /*
          Jump stitch.
        */
        y=((y & 0x0f) << 8)+ReadBlobByte(image);
        if ((y & 0x800) != 0)
          y-=0x1000;
      }
    /*
      Note stitch (x,y).
    */
    x+=delta_x;
    y+=delta_y;
    delta_x=x;
    delta_y=y;
    stitches[i].x=(double) x;
    stitches[i].y=(double) y;
    if ((double) x < bounds.x1)
      bounds.x1=(double) x;
    if ((double) x > bounds.x2)
      bounds.x2=(double) x;
    if ((double) y < bounds.y1)
      bounds.y1=(double) y;
    if ((double) y > bounds.y2)
      bounds.y2=(double) y;
    i++;
    if (i >= (ssize_t) number_stitches)
      {
        /*
          Make room for more stitches.
        */
        number_stitches<<=1;
        stitches=(PointInfo *)  ResizeQuantumMemory(stitches,(size_t)
          number_stitches,sizeof(*stitches));
        if (stitches == (PointInfo *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
     }
  }
  j++;
  blocks[j].offset=(ssize_t) i;
  number_blocks=(size_t) j;
  image->columns=bounds.x2-bounds.x1;
  image->rows=bounds.y2-bounds.y1;
  status=SetImageExtent(image,image->columns,image->rows,errorinfo);
  if (status == MagickFalse)
    {
      stitches=(PointInfo *) RelinquishMagickMemory(stitches);
      return(DestroyImageList(image));
    }
  /*
    Write stitches as SVG file.
  */
  file=(FILE *) NULL;
  unique_file=AcquireUniqueFileResource(filename);
  if (unique_file != -1)
    file=fdopen(unique_file,"wb");
  if ((unique_file == -1) || (file == (FILE *) NULL))
    ThrowImageException(FileOpenError,"UnableToCreateTemporaryFile");
  (void) FormatLocaleFile(file,"<?xml version=\"1.0\"?>\n");
  (void) FormatLocaleFile(file,"<svg xmlns=\"http://www.w3.org/2000/svg\" "
    "xlink=\"http://www.w3.org/1999/xlink\" "
    "ev=\"http://www.w3.org/2001/xml-events\" version=\"1.1\" "
    "baseProfile=\"full\" width=\"%g\" height=\"%g\">\n",(double)
    image->columns,(double) image->rows);
  for (i=0; i < (ssize_t) number_blocks; i++)
  {
    offset=blocks[i].offset;
    (void) FormatLocaleFile(file,"  <path stroke=\"#%02x%02x%02x\" "
      "fill=\"none\" d=\"M %g %g",blocks[i].color->red,blocks[i].color->green,
      blocks[i].color->blue,stitches[offset].x-bounds.x1,
      stitches[offset].y-bounds.y1);
    for (j=1; j < (ssize_t) (blocks[i+1].offset-offset); j++)
      (void) FormatLocaleFile(file," L %g %g",stitches[offset+j].x-bounds.x1,
        stitches[offset+j].y-bounds.y1);
    (void) FormatLocaleFile(file,"\"/>\n");
  }
  (void) FormatLocaleFile(file,"</svg>\n");
  (void) fclose(file);
  stitches=(PointInfo *) RelinquishMagickMemory(stitches);
  (void) CloseBlob(image);
  image=DestroyImage(image);
  /*
    Read SVG file.
  */
  read_info=CloneImageInfo(image_info);
  SetImageInfoBlob(read_info,(void *) NULL,0);
  (void) FormatLocaleString(read_info->filename,MagickPathExtent,"svg:%s",
    filename);
  image=ReadImage(read_info,errorinfo);
  if (image != (Image *) NULL)
    {
      (void) CopyMagickString(image->filename,image_info->filename,
        MagickPathExtent);
      (void) CopyMagickString(image->magick_filename,image_info->filename,
        MagickPathExtent);
      (void) CopyMagickString(image->magick,"PES",MagickPathExtent);
    }
  read_info=DestroyImageInfo(read_info);
  (void) RelinquishUniqueFileResource(filename);
  return(GetFirstImageInList(image));
}