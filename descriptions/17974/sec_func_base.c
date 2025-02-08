static boolean ReadProfile(j_decompress_ptr jpeg_info)
{
  char
    name[MagickPathExtent];

  const StringInfo
    *previous_profile;

  ErrorManager
    *error_manager;

  ExceptionInfo
    *exception;

  Image
    *image;

  int
    marker;

  MagickBooleanType
    status;

  register ssize_t
    i;

  register unsigned char
    *p;

  size_t
    length;

  StringInfo
    *profile;

  /*
    Read generic profile.
  */
  length=(size_t) ((size_t) GetCharacter(jpeg_info) << 8);
  length+=(size_t) GetCharacter(jpeg_info);
  if (length <= 2)
    return(TRUE);
  length-=2;
  marker=jpeg_info->unread_marker-JPEG_APP0;
  (void) FormatLocaleString(name,MagickPathExtent,"APP%d",marker);
  error_manager=(ErrorManager *) jpeg_info->client_data;
  exception=error_manager->exception;
  image=error_manager->image;
  profile=BlobToStringInfo((const void *) NULL,length);
  if (profile == (StringInfo *) NULL)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(FALSE);
    }
  error_manager->profile=profile;
  p=GetStringInfoDatum(profile);
  for (i=0; i < (ssize_t) length; i++)
  {
    int
      c;

    c=GetCharacter(jpeg_info);
    if (c == EOF)
      break;
    *p++=(unsigned char) c;
  }
  error_manager->profile=NULL;
  if (i != (ssize_t) length)
    {
      profile=DestroyStringInfo(profile);
      (void) ThrowMagickException(exception,GetMagickModule(),
        CorruptImageError,"InsufficientImageDataInFile","`%s'",
        image->filename);
      return(FALSE);
    }
  if (marker == 1)
    {
      p=GetStringInfoDatum(profile);
      if ((length > 4) && (LocaleNCompare((char *) p,"exif",4) == 0))
        (void) CopyMagickString(name,"exif",MagickPathExtent);
      else
        if ((length > XmpNamespaceExtent) &&
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
  status=SetImageProfile(image,name,profile,exception);
  profile=DestroyStringInfo(profile);
  if (status == MagickFalse)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
      return(FALSE);
    }
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "Profile: %s, %.20g bytes",name,(double) length);
  return(TRUE);
}