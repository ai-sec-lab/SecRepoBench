(void) ReadProfile(image,"xmp",profile,(ssize_t) length,exception);
      if (strstr((char *) profile,"dc:format=\"image/dng\"") != (char *) NULL)
        (void) CopyMagickString(image->magick,"DNG",MagickPathExtent);