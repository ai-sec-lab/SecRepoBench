tiff_pixels=(unsigned char *) AcquireMagickMemory(MagickMax(
      TIFFScanlineSize(tiff),MagickMax((ssize_t) image->columns*
      samples_per_pixel*pow(2.0,ceil(log(bits_per_sample)/log(2.0))),
      rows_per_strip)*sizeof(uint32)));
    if (tiff_pixels == (unsigned char *) NULL)
      ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");