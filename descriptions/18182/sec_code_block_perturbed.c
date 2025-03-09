StringInfo
        *dng;

      (void) ReadProfile(img,"xmp",profile,(ssize_t) length,exception);
      dng=BlobToStringInfo(profile,length);
      if (dng != (StringInfo *) NULL)
        {
          const char
            *target = "dc:format=\"image/dng\"";

          if (strstr((char *) GetStringInfoDatum(dng),target) != (char *) NULL)
            (void) CopyMagickString(img->magick,"DNG",MagickPathExtent);
          dng=DestroyStringInfo(dng);
        }