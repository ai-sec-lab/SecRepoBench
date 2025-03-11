static const char *GetICCProperty(const Image *image,const char *key,
  ExceptionInfo *exception_info)
{
  const char
    *value;

  const StringInfo
    *profile;

  /*
    Return ICC profile property.
  */
  if (image->properties != (void *) NULL)
    {
      value=(const char *) GetValueFromSplayTree((SplayTreeInfo *)
        image->properties,key);
      if (value != (const char *) NULL)
        return(value);
    }
  profile=GetImageProfile(image,"icc");
  if (profile == (StringInfo *) NULL)
    profile=GetImageProfile(image,"icm");
  if (profile == (StringInfo *) NULL)
    return((const char *) NULL);
  if (GetStringInfoLength(profile) < 128)
    return((const char *) NULL);  /* minimum ICC profile length */
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
          (void) SetImageProperty((Image *) image,"icc:name",name,exception_info);
#else
        StringInfo
          *info;

        unsigned int
          extent;

        info=AcquireStringInfo(0);
        extent=cmsGetProfileInfoASCII(icc_profile,cmsInfoDescription,"en","US",
          NULL,0);
        if (extent != 0)
          {
            SetStringInfoLength(info,extent+1);
            extent=cmsGetProfileInfoASCII(icc_profile,cmsInfoDescription,"en",
              "US",(char *) GetStringInfoDatum(info),extent);
            if (extent != 0)
              (void) SetImageProperty((Image *) image,"icc:description",
                (char *) GetStringInfoDatum(info),exception_info);
         }
        extent=cmsGetProfileInfoASCII(icc_profile,cmsInfoManufacturer,"en","US",
          NULL,0);
        if (extent != 0)
          {
            SetStringInfoLength(info,extent+1);
            extent=cmsGetProfileInfoASCII(icc_profile,cmsInfoManufacturer,"en",
              "US",(char *) GetStringInfoDatum(info),extent);
            if (extent != 0)
              (void) SetImageProperty((Image *) image,"icc:manufacturer",
                (char *) GetStringInfoDatum(info),exception_info);
          }
        extent=cmsGetProfileInfoASCII(icc_profile,cmsInfoModel,"en","US",
          NULL,0);
        if (extent != 0)
          {
            SetStringInfoLength(info,extent+1);
            extent=cmsGetProfileInfoASCII(icc_profile,cmsInfoModel,"en","US",
              (char *) GetStringInfoDatum(info),extent);
            if (extent != 0)
              (void) SetImageProperty((Image *) image,"icc:model",
                (char *) GetStringInfoDatum(info),exception_info);
          }
        extent=cmsGetProfileInfoASCII(icc_profile,cmsInfoCopyright,"en","US",
          NULL,0);
        if (extent != 0)
          {
            SetStringInfoLength(info,extent+1);
            extent=cmsGetProfileInfoASCII(icc_profile,cmsInfoCopyright,"en",
              "US",(char *) GetStringInfoDatum(info),extent);
            if (extent != 0)
              (void) SetImageProperty((Image *) image,"icc:copyright",
                (char *) GetStringInfoDatum(info),exception_info);
          }
        info=DestroyStringInfo(info);
#endif
        (void) cmsCloseProfile(icc_profile);
      }
  }
#endif
  if (image->properties != (void *) NULL)
    {
      value=(const char *) GetValueFromSplayTree((SplayTreeInfo *)
        image->properties,key);
      if (value != (const char *) NULL)
        return(value);
    }
  return((const char *) NULL);
}