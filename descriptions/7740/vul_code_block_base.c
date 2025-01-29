(void) cmsGetProfileInfoASCII(icc_profile,cmsInfoDescription,"en","US",
          info,MagickPathExtent);
        (void) SetImageProperty((Image *) image,"icc:description",info,
          exception);
        (void) cmsGetProfileInfoASCII(icc_profile,cmsInfoManufacturer,"en","US",
          info,MagickPathExtent);
        (void) SetImageProperty((Image *) image,"icc:manufacturer",info,
          exception);
        (void) cmsGetProfileInfoASCII(icc_profile,cmsInfoModel,"en","US",info,
          MagickPathExtent);
        (void) SetImageProperty((Image *) image,"icc:model",info,exception);
        (void) cmsGetProfileInfoASCII(icc_profile,cmsInfoCopyright,"en","US",
          info,MagickPathExtent);
        (void) SetImageProperty((Image *) image,"icc:copyright",info,exception);