if ((runlength == (unsigned char *) NULL) ||
      (buffer == (unsigned char *) NULL) ||
      (scanline == (unsigned char *) NULL))
    {
      if (runlength != (unsigned char *) NULL)
        runlength=(unsigned char *) RelinquishMagickMemory(runlength);
      if (buffer != (unsigned char *) NULL)
        buffer=(unsigned char *) RelinquishMagickMemory(buffer);
      if (scanline != (unsigned char *) NULL)
        scanline=(unsigned char *) RelinquishMagickMemory(scanline);
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    }
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    (void) TransformImageColorspace(image,sRGBColorspace,exception);
  /*
    Convert to GRAY raster scanline.
  */