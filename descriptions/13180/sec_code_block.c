if ((TIFFGetField(tiff,TIFFTAG_ARTIST,&text) == 1) &&
      (text != (char *) NULL))
    (void) SetImageProperty(image,"tiff:artist",text,exception);