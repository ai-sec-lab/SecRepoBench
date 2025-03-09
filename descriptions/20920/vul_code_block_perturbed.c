if (TIFFIsByteSwapped(tiff) != 0)
        TIFFSwabArrayOfLong((uint32 *) profile,(size_t) length);
      status=ReadProfile(img,"iptc",profile,4L*length,exception);