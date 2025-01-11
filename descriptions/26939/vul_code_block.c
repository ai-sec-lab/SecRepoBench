register unsigned char
    *p;

  size_t
    length;

  /*
    Detect corrupt profiles and if discovered, repair.
  */
  if (LocaleCompare(name,"xmp") == 0)
    {
      /*
        Remove garbage after xpacket end.
      */
      p=GetStringInfoDatum(profile);
      p=(unsigned char *) strstr((const char *) p,"<?xpacket end=\"w\"?>");
      if (p != (unsigned char *) NULL)
        {
          p+=19;
          length=p-GetStringInfoDatum(profile);
          if (length != GetStringInfoLength(profile))
            {
              *p='\0';
              SetStringInfoLength(profile,length);
              return(MagickTrue);
            }
        }
      return(MagickFalse);
    }
  if (LocaleCompare(name,"exif") == 0)
    {
      /*
        Check if profile starts with byte order marker instead of Exif.
      */
      p=GetStringInfoDatum(profile);
      if ((LocaleNCompare((const char *) p,"MM",2) == 0) ||
          (LocaleNCompare((const char *) p,"II",2) == 0))
        {
          const unsigned char
            profile_start[] = "Exif\0\0";

          StringInfo
            *exif_profile;

          exif_profile=AcquireStringInfo(GetStringInfoLength(profile)+6);
          if (exif_profile != (StringInfo *) NULL)
            {
              SetStringInfoDatum(exif_profile,profile_start);
              ConcatenateStringInfo(exif_profile,profile);
              SetStringInfoLength(profile,GetStringInfoLength(exif_profile));
              SetStringInfo(profile,exif_profile);
              exif_profile=DestroyStringInfo(exif_profile);
              return(MagickTrue);
            }
        }
    }
  return(MagickFalse);