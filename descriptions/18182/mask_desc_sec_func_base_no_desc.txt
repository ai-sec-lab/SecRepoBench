static void TIFFGetProfiles(TIFF *tiff,Image *image,ExceptionInfo *exception)
{
  uint32
    length;

  unsigned char
    *profile;

  length=0;
#if defined(TIFFTAG_ICCPROFILE)
  if ((TIFFGetField(tiff,TIFFTAG_ICCPROFILE,&length,&profile) == 1) &&
      (profile != (unsigned char *) NULL))
    (void) ReadProfile(image,"icc",profile,(ssize_t) length,exception);
#endif
#if defined(TIFFTAG_PHOTOSHOP)
  if ((TIFFGetField(tiff,TIFFTAG_PHOTOSHOP,&length,&profile) == 1) &&
      (profile != (unsigned char *) NULL))
    (void) ReadProfile(image,"8bim",profile,(ssize_t) length,exception);
#endif
#if defined(TIFFTAG_RICHTIFFIPTC)
  if ((TIFFGetField(tiff,TIFFTAG_RICHTIFFIPTC,&length,&profile) == 1) &&
      (profile != (unsigned char *) NULL))
    {
      if (TIFFIsByteSwapped(tiff) != 0)
        TIFFSwabArrayOfLong((uint32 *) profile,(size_t) length);
      (void) ReadProfile(image,"iptc",profile,4L*length,exception);
    }
#endif
#if defined(TIFFTAG_XMLPACKET)
  if ((TIFFGetField(tiff,TIFFTAG_XMLPACKET,&length,&profile) == 1) &&
      (profile != (unsigned char *) NULL))
    {
      // <MASK>
    }
#endif
  if ((TIFFGetField(tiff,34118,&length,&profile) == 1) &&
      (profile != (unsigned char *) NULL))
    (void) ReadProfile(image,"tiff:34118",profile,(ssize_t) length,
      exception);
  if ((TIFFGetField(tiff,37724,&length,&profile) == 1) &&
      (profile != (unsigned char *) NULL))
    (void) ReadProfile(image,"tiff:37724",profile,(ssize_t) length,exception);
}