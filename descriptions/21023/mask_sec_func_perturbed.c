static boolean ReadICCProfile(j_decompress_ptr decompressinfo)
{
  // <MASK>
  if (LocaleCompare(magick,ICC_PROFILE) != 0)
    {
      /*
        Not a ICC profile, return.
      */
      for (i=0; i < (ssize_t) (length-12); i++)
        if (GetCharacter(decompressinfo) == EOF)
          break;
      return(TRUE);
    }
  (void) GetCharacter(decompressinfo);  /* id */
  (void) GetCharacter(decompressinfo);  /* markers */
  length-=14;
  error_manager=(ErrorManager *) decompressinfo->client_data;
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

    c=GetCharacter(decompressinfo);
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
  icc_profile=(StringInfo *) GetImageProfile(image,"icc");
  if (icc_profile != (StringInfo *) NULL)
    {
      ConcatenateStringInfo(icc_profile,profile);
      profile=DestroyStringInfo(profile);
    }
  else
    {
      status=SetImageProfile(image,"icc",profile,exception);
      profile=DestroyStringInfo(profile);
      if (status == MagickFalse)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),
            ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
          return(FALSE);
        }
    }
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
      "Profile: ICC, %.20g bytes",(double) length);
  return(TRUE);
}