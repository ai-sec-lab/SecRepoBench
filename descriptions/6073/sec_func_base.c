static MagickBooleanType Sync8BimProfile(Image *image,StringInfo *profile)
{
  size_t
    length;

  ssize_t
    count;

  unsigned char
    *p;

  unsigned short
    id;

  length=GetStringInfoLength(profile);
  p=GetStringInfoDatum(profile);
  while (length != 0)
  {
    if (ReadProfileByte(&p,&length) != 0x38)
      continue;
    if (ReadProfileByte(&p,&length) != 0x42)
      continue;
    if (ReadProfileByte(&p,&length) != 0x49)
      continue;
    if (ReadProfileByte(&p,&length) != 0x4D)
      continue;
    if (length < 7)
      return(MagickFalse);
    id=ReadProfileMSBShort(&p,&length);
    count=(ssize_t) ReadProfileByte(&p,&length);
    if ((count >= (ssize_t) length) || (count < 0))
      return(MagickFalse);
    p+=count;
    length-=count;
    if ((*p & 0x01) == 0)
      (void) ReadProfileByte(&p,&length);
    count=(ssize_t) ReadProfileMSBLong(&p,&length);
    if ((count > (ssize_t) length) || (count < 0))
      return(MagickFalse);
    if ((id == 0x3ED) && (count == 16))
      {
        if (image->units == PixelsPerCentimeterResolution)
          WriteProfileLong(MSBEndian,(unsigned int) (image->resolution.x*2.54*
            65536.0),p);
        else
          WriteProfileLong(MSBEndian,(unsigned int) (image->resolution.x*
            65536.0),p);
        WriteProfileShort(MSBEndian,(unsigned short) image->units,p+4);
        if (image->units == PixelsPerCentimeterResolution)
          WriteProfileLong(MSBEndian,(unsigned int) (image->resolution.y*2.54*
            65536.0),p+8);
        else
          WriteProfileLong(MSBEndian,(unsigned int) (image->resolution.y*
            65536.0),p+8);
        WriteProfileShort(MSBEndian,(unsigned short) image->units,p+12);
      }
    p+=count;
    length-=count;
  }
  return(MagickTrue);
}