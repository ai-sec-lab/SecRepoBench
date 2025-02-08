if (marker == 1)
    {
      p=GetStringInfoDatum(profile);
      if ((profilelength > 4) && (LocaleNCompare((char *) p,"exif",4) == 0))
        (void) CopyMagickString(name,"exif",MagickPathExtent);
      else
        if ((profilelength > XmpNamespaceExtent) &&
            (LocaleNCompare((char *) p,xmp_namespace,XmpNamespaceExtent-1) == 0))
          {
            ssize_t
              j;

            /*
              Extract namespace from XMP profile.
            */
            p=GetStringInfoDatum(profile)+XmpNamespaceExtent;
            for (j=XmpNamespaceExtent; j < (ssize_t) GetStringInfoLength(profile); j++)
            {
              if (*p == '\0')
                break;
              p++;
            }
            if (j < (ssize_t) GetStringInfoLength(profile))
              (void) DestroyStringInfo(SplitStringInfo(profile,(size_t) (j+1)));
            (void) CopyMagickString(name,"xmp",MagickPathExtent);
          }
    }
  previous_profile=GetImageProfile(image,name);
  if ((previous_profile != (const StringInfo *) NULL) &&
      (CompareStringInfo(previous_profile,profile) != 0))
    {
      size_t
        profile_length;

      profile_length=GetStringInfoLength(profile);
      SetStringInfoLength(profile,GetStringInfoLength(profile)+
        GetStringInfoLength(previous_profile));
      (void) memmove(GetStringInfoDatum(profile)+
        GetStringInfoLength(previous_profile),GetStringInfoDatum(profile),
        profile_length);
      (void) memcpy(GetStringInfoDatum(profile),
        GetStringInfoDatum(previous_profile),
        GetStringInfoLength(previous_profile));
      GetStringInfoDatum(profile)[GetStringInfoLength(profile)]='\0';
    }