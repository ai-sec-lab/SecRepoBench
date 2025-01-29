status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  (void) SetImageBackgroundColor(image,exception);
  for (n=0; n < (ssize_t) dpx.image.number_elements; n++)
  {
    /*
      Convert DPX raster image to pixel packets.
    */
    if ((dpx.image.image_element[n].data_offset != ~0U) &&
        (dpx.image.image_element[n].data_offset != 0U))
      {
         MagickOffsetType
           data_offset;

         data_offset=(MagickOffsetType) dpx.image.image_element[n].data_offset;
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
      dpx.image.image_element[n].colorimetric,&image->chromaticity);
    image->depth=dpx.image.image_element[n].bit_size;
    if ((image->depth == 0) || (image->depth > 32))
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    samples_per_pixel=1;
    quantum_type=GrayQuantum;
    component_type=dpx.image.image_element[n].descriptor;
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
        if (dpx.image.image_element[n].transfer_characteristic == LogarithmicColorimetric)
          SetImageColorspace(image,LogColorspace,exception);
        if (dpx.image.image_element[n].transfer_characteristic == PrintingDensityColorimetric)
          SetImageColorspace(image,LogColorspace,exception);
        break;
      }
    }
    extent=GetBytesPerRow(image->columns,samples_per_pixel,image->depth,
      dpx.image.image_element[n].packing == 0 ? MagickFalse : MagickTrue);
    /*
      DPX any-bit pixel format.
    */
    row=0;
    quantum_info=AcquireQuantumInfo(image_info,image);
    if (quantum_info == (QuantumInfo *) NULL)
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    SetQuantumQuantum(quantum_info,32);
    SetQuantumPack(quantum_info,dpx.image.image_element[n].packing == 0 ?
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
    if ((i+1) < (ssize_t) dpx.image.number_elements)
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