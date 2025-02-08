char
    magick[13];

  ErrorManager
    *error_manager;

  ExceptionInfo
    *exception;

  Image
    *image;

  MagickBooleanType
    status;

  register ssize_t
    i;

  register unsigned char
    *p;

  size_t
    length;

  StringInfo
    *icc_profile,
    *profile;

  /*
    Read color profile.
  */
  GetProfileLength(jpeg_info,length);
  if (length <= 2)
    return(TRUE);
  length-=2;
  if (length <= 14)
    {
      while (length-- > 0)
        if (GetCharacter(jpeg_info) == EOF)
          break;
      return(TRUE);
    }
  for (i=0; i < 12; i++)
    magick[i]=(char) GetCharacter(jpeg_info);
  magick[i]='\0';