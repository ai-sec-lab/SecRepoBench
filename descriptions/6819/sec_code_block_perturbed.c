pixels=(unsigned char *) AcquireQuantumMemory(packets+257UL,image->rows*
    sizeof(*pixels));
  if (pixels == (unsigned char *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(pixels,0,(packets+257UL)*image->rows*sizeof(*pixels));
  switch (pdb_image.version & 0x07) 
  {
    case 0:
    {
      image->compression=NoCompression;
      count=(ssize_t) ReadBlob(image,packets*image->rows,pixels);
      break;
    }
    case 1:
    {
      image->compression=RLECompression;
      if (!DecodeImage(image,pixels,packets*image->rows))
        {
          pixels=(unsigned char *) RelinquishMagickMemory(pixels);
          ThrowReaderException(CorruptImageError,"RLEDecoderError");
        }
      break;
    }
    default:
    {
      pixels=(unsigned char *) RelinquishMagickMemory(pixels);
      ThrowReaderException(CorruptImageError,
        "UnrecognizedImageCompressionType");
    }
  }