static Image *ReadPCXImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
#define ThrowPCXException(severity,tag) \
{ \
  if (scanline != (unsigned char *) NULL) \
    scanline=(unsigned char *) RelinquishMagickMemory(scanline); \
  if (pixel_info != (MemoryInfo *) NULL) \
    pixel_info=RelinquishVirtualMemory(pixel_info); \
  if (page_table != (MagickOffsetType *) NULL) \
    page_table=(MagickOffsetType *) RelinquishMagickMemory(page_table); \
  ThrowReaderException(severity,tag); \
}

  Image
    *image;

  int
    bitmask,
    id,
    mask;

  MagickBooleanType
    status;

  MagickOffsetType
    offset,
    *page_table;

  MemoryInfo
    *pixel_info;

  PCXInfo
    pcx_info;

  register ssize_t
    x;

  register Quantum
    *q;

  register ssize_t
    i;

  register unsigned char
    *p,
    *r;

  size_t
    one,
    pcx_packets;

  ssize_t
    count,
    y;

  unsigned char
    packet,
    pcx_colormap[768],
    *pixels,
    *scanline;

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
    Determine if this a PCX file.
  */
  page_table=(MagickOffsetType *) NULL;
  scanline=(unsigned char *) NULL;
  pixel_info=(MemoryInfo *) NULL;
  if (LocaleCompare(image_info->magick,"DCX") == 0)
    {
      size_t
        magic;

      /*
        Read the DCX page table.
      */
      magic=ReadBlobLSBLong(image);
      if (magic != 987654321)
        ThrowPCXException(CorruptImageError,"ImproperImageHeader");
      page_table=(MagickOffsetType *) AcquireQuantumMemory(1024UL,
        sizeof(*page_table));
      if (page_table == (MagickOffsetType *) NULL)
        ThrowPCXException(ResourceLimitError,"MemoryAllocationFailed");
      for (id=0; id < 1024; id++)
      {
        page_table[id]=(MagickOffsetType) ReadBlobLSBLong(image);
        if (page_table[id] == 0)
          break;
      }
    }
  if (page_table != (MagickOffsetType *) NULL)
    {
      offset=SeekBlob(image,(MagickOffsetType) page_table[0],SEEK_SET);
      if (offset < 0)
        ThrowPCXException(CorruptImageError,"ImproperImageHeader");
    }
  count=ReadBlob(image,1,&pcx_info.identifier);
  for (id=1; id < 1024; id++)
  {
    int
      bits_per_pixel;

    /*
      Verify PCX identifier.
    */
    pcx_info.version=(unsigned char) ReadBlobByte(image);
    if ((count != 1) || (pcx_info.identifier != 0x0a))
      ThrowPCXException(CorruptImageError,"ImproperImageHeader");
    pcx_info.encoding=(unsigned char) ReadBlobByte(image);
    bits_per_pixel=ReadBlobByte(image);
    if (bits_per_pixel == -1)
      ThrowPCXException(CorruptImageError,"ImproperImageHeader");
    pcx_info.bits_per_pixel=(unsigned char) bits_per_pixel;
    pcx_info.left=ReadBlobLSBShort(image);
    pcx_info.top=ReadBlobLSBShort(image);
    pcx_info.right=ReadBlobLSBShort(image);
    pcx_info.bottom=ReadBlobLSBShort(image);
    pcx_info.horizontal_resolution=ReadBlobLSBShort(image);
    pcx_info.vertical_resolution=ReadBlobLSBShort(image);
    /*
      Read PCX raster colormap.
    */
    image->columns=(size_t) MagickAbsoluteValue((ssize_t) pcx_info.right-
      pcx_info.left)+1UL;
    image->rows=(size_t) MagickAbsoluteValue((ssize_t) pcx_info.bottom-
      pcx_info.top)+1UL;
    if ((image->columns == 0) || (image->rows == 0) ||
        ((pcx_info.bits_per_pixel != 1) && (pcx_info.bits_per_pixel != 2) &&
         (pcx_info.bits_per_pixel != 4) && (pcx_info.bits_per_pixel != 8)))
      ThrowPCXException(CorruptImageError,"ImproperImageHeader");
    image->depth=pcx_info.bits_per_pixel;
    image->units=PixelsPerInchResolution;
    image->resolution.x=(double) pcx_info.horizontal_resolution;
    image->resolution.y=(double) pcx_info.vertical_resolution;
    image->colors=16;
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status == MagickFalse)
      ThrowPCXException(exception->severity,exception->reason);
    count=ReadBlob(image,3*image->colors,pcx_colormap);
    if (count != (ssize_t) (3*image->colors))
      ThrowPCXException(CorruptImageError,"ImproperImageHeader");
    pcx_info.reserved=(unsigned char) ReadBlobByte(image);
    pcx_info.planes=(unsigned char) ReadBlobByte(image);
    if (pcx_info.planes == 0)
      ThrowPCXException(CorruptImageError,"ImproperImageHeader");
    if (pcx_info.planes > 6)
      ThrowPCXException(CorruptImageError,"ImproperImageHeader");
    if ((pcx_info.bits_per_pixel*pcx_info.planes) >= 64)
      ThrowPCXException(CorruptImageError,"ImproperImageHeader");
    one=1;
    if ((pcx_info.bits_per_pixel != 8) || (pcx_info.planes == 1))
      if ((pcx_info.version == 3) || (pcx_info.version == 5) ||
          ((pcx_info.bits_per_pixel*pcx_info.planes) == 1))
        image->colors=(size_t) MagickMin(one << (1UL*
          (pcx_info.bits_per_pixel*pcx_info.planes)),256UL);
    if (AcquireImageColormap(image,image->colors,exception) == MagickFalse)
      ThrowPCXException(ResourceLimitError,"MemoryAllocationFailed");
    if ((pcx_info.bits_per_pixel >= 8) && (pcx_info.planes != 1))
      image->storage_class=DirectClass;
    p=pcx_colormap;
    for (i=0; i < (ssize_t) image->colors; i++)
    {
      image->colormap[i].red=ScaleCharToQuantum(*p++);
      image->colormap[i].green=ScaleCharToQuantum(*p++);
      image->colormap[i].blue=ScaleCharToQuantum(*p++);
    }
    (void) SetImageBackgroundColor(image,exception);
    pcx_info.bytes_per_line=ReadBlobLSBShort(image);
    pcx_info.palette_info=ReadBlobLSBShort(image);
    pcx_info.horizontal_screensize=ReadBlobLSBShort(image);
    pcx_info.vertical_screensize=ReadBlobLSBShort(image);
    for (i=0; i < 54; i++)
      (void) ReadBlobByte(image);
    /*
      Read image data.
    */
    if (HeapOverflowSanityCheck(image->rows, (size_t) pcx_info.bytes_per_line) != MagickFalse)
      ThrowPCXException(CorruptImageError,"ImproperImageHeader");
    pcx_packets=(size_t) image->rows*pcx_info.bytes_per_line;
    if (HeapOverflowSanityCheck(pcx_packets, (size_t) pcx_info.planes) != MagickFalse)
      ThrowPCXException(CorruptImageError,"ImproperImageHeader");
    if ((pcx_packets/8) > GetBlobSize(image))
      ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
    pcx_packets=(size_t) pcx_packets*pcx_info.planes;
    if ((size_t) (pcx_info.bits_per_pixel*pcx_info.planes*image->columns) >
        (pcx_packets*8U))
      ThrowPCXException(CorruptImageError,"ImproperImageHeader");
    scanline=(unsigned char *) AcquireQuantumMemory(MagickMax(image->columns,
      pcx_info.bytes_per_line),MagickMax(8,pcx_info.planes)*sizeof(*scanline));
    pixel_info=AcquireVirtualMemory(pcx_packets,2*sizeof(*pixels));
    if ((scanline == (unsigned char *) NULL) ||
        (pixel_info == (MemoryInfo *) NULL))
      {
        if (scanline != (unsigned char *) NULL)
          scanline=(unsigned char *) RelinquishMagickMemory(scanline);
        if (pixel_info != (MemoryInfo *) NULL)
          pixel_info=RelinquishVirtualMemory(pixel_info);
        ThrowPCXException(ResourceLimitError,"MemoryAllocationFailed");
      }
    // <MASK>
  }
  if (page_table != (MagickOffsetType *) NULL)
    page_table=(MagickOffsetType *) RelinquishMagickMemory(page_table);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}