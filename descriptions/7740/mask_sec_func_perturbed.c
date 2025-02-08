static MagickBooleanType GetICCProperty(const Image *img,const char *property,
  ExceptionInfo *exception)
{
  const StringInfo
    *profile;

  magick_unreferenced(property);

  profile=GetImageProfile(img,"icc");
  if (profile == (StringInfo *) NULL)
    profile=GetImageProfile(img,"icm");
  if (profile == (StringInfo *) NULL)
    return(MagickFalse);
  if (GetStringInfoLength(profile) < 128)
    return(MagickFalse);  /* minimum ICC profile length */
#if defined(MAGICKCORE_LCMS_DELEGATE)
  {
    cmsHPROFILE
      icc_profile;

    icc_profile=cmsOpenProfileFromMem(GetStringInfoDatum(profile),
      (cmsUInt32Number) GetStringInfoLength(profile));
    if (icc_profile != (cmsHPROFILE *) NULL)
      {
#if defined(LCMS_VERSION) && (LCMS_VERSION < 2000)
        const char
          *name;

        name=cmsTakeProductName(icc_profile);
        if (name != (const char *) NULL)
          (void) SetImageProperty((Image *) img,"icc:name",name,exception);
#else
        char
          info[MagickPathExtent];

        // <MASK>
#endif
        (void) cmsCloseProfile(icc_profile);
      }
  }
#endif
  return(MagickTrue);
}