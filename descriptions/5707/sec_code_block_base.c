if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  header=0;
  if (ReadBlob(image,2,(unsigned char *) &header) == 0)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  if (header != 0)
    ThrowReaderException(CoderError,"OnlyLevelZerofilesSupported");