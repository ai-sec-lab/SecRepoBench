if (cmsGetProfileInfoASCII(icc_profile,cmsInfoDescription,"en","US",
              info,MagickPathExtent) != 0)
          (void) SetImageProperty((Image *) image,"icc:description",info,
            exception);
        if (cmsGetProfileInfoASCII(icc_profile,cmsInfoManufacturer,"en","US",
              info,MagickPathExtent) != 0)
          (void) SetImageProperty((Image *) image,"icc:manufacturer",info,
            exception);
        if (cmsGetProfileInfoASCII(icc_profile,cmsInfoModel,"en","US",info,
              MagickPathExtent) != 0)
          (void) SetImageProperty((Image *) image,"icc:model",info,exception);
        if (cmsGetProfileInfoASCII(icc_profile,cmsInfoCopyright,"en","US",
              info,MagickPathExtent) != 0)
          (void) SetImageProperty((Image *) image,"icc:copyright",info,
            exception);