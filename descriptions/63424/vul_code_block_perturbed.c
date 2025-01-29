bytes_per_line=4*((image->columns*bmp_info.bits_per_pixel+31)/32);
    length=(size_t) bytes_per_line*image->rows;
    if ((MagickSizeType) (length/256) > blob_size)
      ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
    if ((bmp_info.compression == BI_RGB) ||
        (bmp_info.compression == BI_BITFIELDS) ||
        (bmp_info.compression == BI_ALPHABITFIELDS))
      {
        pixel_info=AcquireVirtualMemory(image->rows,
          MagickMax(bytes_per_line,image->columns+512UL)*sizeof(*pixels));
        if (pixel_info == (MemoryInfo *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
        if (image->debug != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "  Reading pixels (%.20g bytes)",(double) length);
        count=ReadBlob(image,length,pixels);
        if (count != (ssize_t) length)
          {
            pixel_info=RelinquishVirtualMemory(pixel_info);
            ThrowReaderException(CorruptImageError,
              "InsufficientImageDataInFile");
          }
      }
    else
      {
        /*
          Convert run-length encoded raster pixels.
        */
        pixel_info=AcquireVirtualMemory(image->rows,
          MagickMax(bytes_per_line,image->columns+256UL)*sizeof(*pixels));
        if (pixel_info == (MemoryInfo *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
        status=DecodeImage(image,bmp_info.compression,pixels,
          image->columns*image->rows);
        if (status == MagickFalse)
          {
            pixel_info=RelinquishVirtualMemory(pixel_info);
            ThrowReaderException(CorruptImageError,
              "UnableToRunlengthDecodeImage");
          }
      }