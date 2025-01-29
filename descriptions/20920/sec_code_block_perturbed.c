const TIFFField
        *field;

      if (TIFFIsByteSwapped(tiff) != 0)
        TIFFSwabArrayOfLong((uint32 *) profile,(size_t) length);
      field=TIFFFieldWithTag(tiff,TIFFTAG_RICHTIFFIPTC);
      if (TIFFFieldDataType(field) == TIFF_LONG)
        status=ReadProfile(img,"iptc",profile,4L*length,exception);
      else
        status=ReadProfile(img,"iptc",profile,length,exception);