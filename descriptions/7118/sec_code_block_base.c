rows_per_strip=image->columns*image->rows;
    if (TIFFGetField(tiff,TIFFTAG_ROWSPERSTRIP,&rows_per_strip) == 1)
      {
        char
          value[MagickPathExtent];

        method=ReadStripMethod;
        (void) FormatLocaleString(value,MagickPathExtent,"%u",
          (unsigned int) rows_per_strip);
        (void) SetImageProperty(image,"tiff:rows-per-strip",value,exception);
      }
    if (rows_per_strip > (image->columns*image->rows))
      ThrowTIFFException(CorruptImageError,"ImproperImageHeader");