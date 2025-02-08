static Image *ReadVIFFImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
#define VFF_CM_genericRGB  15
#define VFF_CM_ntscRGB  1
#define VFF_CM_NONE  0
#define VFF_DEP_DECORDER  0x4
#define VFF_DEP_NSORDER  0x8
#define VFF_DES_RAW  0
#define VFF_LOC_IMPLICIT  1
#define VFF_MAPTYP_NONE  0
#define VFF_MAPTYP_1_BYTE  1
#define VFF_MAPTYP_2_BYTE  2
#define VFF_MAPTYP_4_BYTE  4
#define VFF_MAPTYP_FLOAT  5
#define VFF_MAPTYP_DOUBLE  7
#define VFF_MS_NONE  0
#define VFF_MS_ONEPERBAND  1
#define VFF_MS_SHARED  3
#define VFF_TYP_BIT  0
#define VFF_TYP_1_BYTE  1
#define VFF_TYP_2_BYTE  2
#define VFF_TYP_4_BYTE  4
#define VFF_TYP_FLOAT  5
#define VFF_TYP_DOUBLE  9

  typedef struct _ViffInfo
  {
    unsigned char
      identifier,
      file_type,
      release,
      version,
      machine_dependency,
      reserve[3];

    char
      comment[512];

    unsigned int
      rows,
      columns,
      subrows;

    int
      x_offset,
      y_offset;

    float
      x_bits_per_pixel,
      y_bits_per_pixel;

    unsigned int
      location_type,
      location_dimension,
      number_of_images,
      number_data_bands,
      data_storage_type,
      data_encode_scheme,
      map_scheme,
      map_storage_type,
      map_rows,
      map_columns,
      map_subrows,
      map_enable,
      maps_per_cycle,
      color_space_model;
  } ViffInfo;

  double
    min_value,
    scale_factor,
    value;

  Image
    *image;

  int
    bit;

  MagickBooleanType
    status;

  MagickSizeType
    number_pixels;

  register ssize_t
    x;

  register Quantum
    *q;

  register ssize_t
    i;

  register unsigned char
    *p;

  size_t
    bytes_per_pixel,
    max_packets,
    quantum;

  ssize_t
    count,
    y;

  unsigned char
    *pixels;

  unsigned long
    lsb_first;

  ViffInfo
    viff_info;

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
    Read VIFF header (1024 bytes).
  */
  count=ReadBlob(image,1,&viff_info.identifier);
  do
  {
    /*
      Verify VIFF identifier.
    */
    if ((count != 1) || ((unsigned char) viff_info.identifier != 0xab))
      ThrowReaderException(CorruptImageError,"NotAVIFFImage");
    /*
      Initialize VIFF image.
    */
    (void) ReadBlob(image,sizeof(viff_info.file_type),&viff_info.file_type);
    (void) ReadBlob(image,sizeof(viff_info.release),&viff_info.release);
    (void) ReadBlob(image,sizeof(viff_info.version),&viff_info.version);
    (void) ReadBlob(image,sizeof(viff_info.machine_dependency),
      &viff_info.machine_dependency);
    (void) ReadBlob(image,sizeof(viff_info.reserve),viff_info.reserve);
    count=ReadBlob(image,512,(unsigned char *) viff_info.comment);
    if (count != 512)
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    viff_info.comment[511]='\0';
    if (strlen(viff_info.comment) > 4)
      (void) SetImageProperty(image,"comment",viff_info.comment,exception);
    if ((viff_info.machine_dependency == VFF_DEP_DECORDER) ||
        (viff_info.machine_dependency == VFF_DEP_NSORDER))
      image->endian=LSBEndian;
    else
      image->endian=MSBEndian;
    viff_info.rows=ReadBlobLong(image);
    viff_info.columns=ReadBlobLong(image);
    viff_info.subrows=ReadBlobLong(image);
    viff_info.x_offset=ReadBlobSignedLong(image);
    viff_info.y_offset=ReadBlobSignedLong(image);
    viff_info.x_bits_per_pixel=(float) ReadBlobLong(image);
    viff_info.y_bits_per_pixel=(float) ReadBlobLong(image);
    viff_info.location_type=ReadBlobLong(image);
    viff_info.location_dimension=ReadBlobLong(image);
    viff_info.number_of_images=ReadBlobLong(image);
    viff_info.number_data_bands=ReadBlobLong(image);
    viff_info.data_storage_type=ReadBlobLong(image);
    viff_info.data_encode_scheme=ReadBlobLong(image);
    viff_info.map_scheme=ReadBlobLong(image);
    viff_info.map_storage_type=ReadBlobLong(image);
    viff_info.map_rows=ReadBlobLong(image);
    viff_info.map_columns=ReadBlobLong(image);
    viff_info.map_subrows=ReadBlobLong(image);
    viff_info.map_enable=ReadBlobLong(image);
    viff_info.maps_per_cycle=ReadBlobLong(image);
    viff_info.color_space_model=ReadBlobLong(image);
    for (i=0; i < 420; i++)
      (void) ReadBlobByte(image);
    if (EOFBlob(image) != MagickFalse)
      ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
    image->columns=viff_info.rows;
    image->rows=viff_info.columns;
    image->depth=viff_info.x_bits_per_pixel <= 8 ? 8UL :
      MAGICKCORE_QUANTUM_DEPTH;
    image->alpha_trait=viff_info.number_data_bands == 4 ? BlendPixelTrait :
      UndefinedPixelTrait;
    status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status == MagickFalse)
      return(DestroyImageList(image));
    (void) SetImageBackgroundColor(image,exception);
    /*
      Verify that we can read this VIFF image.
    */
    number_pixels=(MagickSizeType) viff_info.columns*viff_info.rows;
    if (number_pixels != (size_t) number_pixels)
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    if (number_pixels == 0)
      ThrowReaderException(CoderError,"ImageColumnOrRowSizeIsNotSupported");
    if ((viff_info.number_data_bands < 1) || (viff_info.number_data_bands > 4))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    if ((viff_info.data_storage_type != VFF_TYP_BIT) &&
        (viff_info.data_storage_type != VFF_TYP_1_BYTE) &&
        (viff_info.data_storage_type != VFF_TYP_2_BYTE) &&
        (viff_info.data_storage_type != VFF_TYP_4_BYTE) &&
        (viff_info.data_storage_type != VFF_TYP_FLOAT) &&
        (viff_info.data_storage_type != VFF_TYP_DOUBLE))
      ThrowReaderException(CoderError,"DataStorageTypeIsNotSupported");
    if (viff_info.data_encode_scheme != VFF_DES_RAW)
      ThrowReaderException(CoderError,"DataEncodingSchemeIsNotSupported");
    if ((viff_info.map_storage_type != VFF_MAPTYP_NONE) &&
        (viff_info.map_storage_type != VFF_MAPTYP_1_BYTE) &&
        (viff_info.map_storage_type != VFF_MAPTYP_2_BYTE) &&
        (viff_info.map_storage_type != VFF_MAPTYP_4_BYTE) &&
        (viff_info.map_storage_type != VFF_MAPTYP_FLOAT) &&
        (viff_info.map_storage_type != VFF_MAPTYP_DOUBLE))
      ThrowReaderException(CoderError,"MapStorageTypeIsNotSupported");
    if ((viff_info.color_space_model != VFF_CM_NONE) &&
        (viff_info.color_space_model != VFF_CM_ntscRGB) &&
        (viff_info.color_space_model != VFF_CM_genericRGB))
      ThrowReaderException(CoderError,"ColorspaceModelIsNotSupported");
    if (viff_info.location_type != VFF_LOC_IMPLICIT)
      ThrowReaderException(CoderError,"LocationTypeIsNotSupported");
    if (viff_info.number_of_images != 1)
      ThrowReaderException(CoderError,"NumberOfImagesIsNotSupported");
    if (viff_info.map_rows == 0)
      viff_info.map_scheme=VFF_MS_NONE;
    switch ((int) viff_info.map_scheme)
    {
      case VFF_MS_NONE:
      {
        if (viff_info.number_data_bands < 3)
          {
            /*
              Create linear color ramp.
            */
            if (viff_info.data_storage_type == VFF_TYP_BIT)
              image->colors=2;
            else
              if (viff_info.data_storage_type == VFF_MAPTYP_1_BYTE)
                image->colors=256UL;
              else
                image->colors=image->depth <= 8 ? 256UL : 65536UL;
            status=AcquireImageColormap(image,image->colors,exception);
            if (status == MagickFalse)
              ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          }
        break;
      }
      case VFF_MS_ONEPERBAND:
      case VFF_MS_SHARED:
      {
        unsigned char
          *viff_colormap;

        /*
          Allocate VIFF colormap.
        */
        switch ((int) viff_info.map_storage_type)
        {
          case VFF_MAPTYP_1_BYTE: bytes_per_pixel=1; break;
          case VFF_MAPTYP_2_BYTE: bytes_per_pixel=2; break;
          case VFF_MAPTYP_4_BYTE: bytes_per_pixel=4; break;
          case VFF_MAPTYP_FLOAT: bytes_per_pixel=4; break;
          case VFF_MAPTYP_DOUBLE: bytes_per_pixel=8; break;
          default: bytes_per_pixel=1; break;
        }
        image->colors=viff_info.map_columns;
        if ((MagickSizeType) (viff_info.map_rows*image->colors) > GetBlobSize(image))
          ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
        if (AcquireImageColormap(image,image->colors,exception) == MagickFalse)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        if ((MagickSizeType) viff_info.map_rows > GetBlobSize(image))
          ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
        if ((MagickSizeType) viff_info.map_rows >
            (viff_info.map_rows*bytes_per_pixel*sizeof(*viff_colormap)))
          ThrowReaderException(CorruptImageError,"ImproperImageHeader");
        viff_colormap=(unsigned char *) AcquireQuantumMemory(image->colors,
          viff_info.map_rows*bytes_per_pixel*sizeof(*viff_colormap));
        if (viff_colormap == (unsigned char *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        /*
          Read VIFF raster colormap.
        */
        count=ReadBlob(image,bytes_per_pixel*image->colors*viff_info.map_rows,
          viff_colormap);
        lsb_first=1;
        if (*(char *) &lsb_first &&
            ((viff_info.machine_dependency != VFF_DEP_DECORDER) &&
             (viff_info.machine_dependency != VFF_DEP_NSORDER)))
          switch ((int) viff_info.map_storage_type)
          {
            case VFF_MAPTYP_2_BYTE:
            {
              MSBOrderShort(viff_colormap,(bytes_per_pixel*image->colors*
                viff_info.map_rows));
              break;
            }
            case VFF_MAPTYP_4_BYTE:
            case VFF_MAPTYP_FLOAT:
            {
              MSBOrderLong(viff_colormap,(bytes_per_pixel*image->colors*
                viff_info.map_rows));
              break;
            }
            default: break;
          }
        for (i=0; i < (ssize_t) (viff_info.map_rows*image->colors); i++)
        {
          switch ((int) viff_info.map_storage_type)
          {
            case VFF_MAPTYP_2_BYTE: value=1.0*((short *) viff_colormap)[i]; break;
            case VFF_MAPTYP_4_BYTE: value=1.0*((int *) viff_colormap)[i]; break;
            case VFF_MAPTYP_FLOAT: value=((float *) viff_colormap)[i]; break;
            case VFF_MAPTYP_DOUBLE: value=((double *) viff_colormap)[i]; break;
            default: value=1.0*viff_colormap[i]; break;
          }
          if (i < (ssize_t) image->colors)
            {
              image->colormap[i].red=(MagickRealType)
                ScaleCharToQuantum((unsigned char) value);
              image->colormap[i].green=(MagickRealType)
                ScaleCharToQuantum((unsigned char) value);
              image->colormap[i].blue=(MagickRealType)
                ScaleCharToQuantum((unsigned char) value);
            }
          else
            if (i < (ssize_t) (2*image->colors))
              image->colormap[i % image->colors].green=(MagickRealType)
                ScaleCharToQuantum((unsigned char) value);
            else
              if (i < (ssize_t) (3*image->colors))
                image->colormap[i % image->colors].blue=(MagickRealType)
                  ScaleCharToQuantum((unsigned char) value);
        }
        viff_colormap=(unsigned char *) RelinquishMagickMemory(viff_colormap);
        break;
      }
      default:
        ThrowReaderException(CoderError,"ColormapTypeNotSupported");
    }
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    /*
      Allocate VIFF pixels.
    */
    switch ((int) viff_info.data_storage_type)
    {
      case VFF_TYP_2_BYTE: bytes_per_pixel=2; break;
      case VFF_TYP_4_BYTE: bytes_per_pixel=4; break;
      case VFF_TYP_FLOAT: bytes_per_pixel=4; break;
      case VFF_TYP_DOUBLE: bytes_per_pixel=8; break;
      default: bytes_per_pixel=1; break;
    }
    if (viff_info.data_storage_type == VFF_TYP_BIT)
      {
        if (HeapOverflowSanityCheck((image->columns+7UL) >> 3UL,image->rows) != MagickFalse)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        max_packets=((image->columns+7UL) >> 3UL)*image->rows;
      }
    else
      {
        if (HeapOverflowSanityCheck((size_t) number_pixels,viff_info.number_data_bands) != MagickFalse)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        max_packets=(size_t) (number_pixels*viff_info.number_data_bands);
      }
    if ((MagickSizeType) (bytes_per_pixel*max_packets) > GetBlobSize(image))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    pixels=(unsigned char *) AcquireQuantumMemory((size_t) MagickMax(
      number_pixels,max_packets),bytes_per_pixel*sizeof(*pixels));
    if (pixels == (unsigned char *) NULL)
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    // <MASK>
  } while ((count != 0) && (viff_info.identifier == 0xab));
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}