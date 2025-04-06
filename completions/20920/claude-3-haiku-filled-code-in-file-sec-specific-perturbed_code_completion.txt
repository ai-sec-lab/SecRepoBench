static MagickBooleanType ReadProfile(Image *image,const char *name,
  const unsigned char *datum,ssize_t length,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  StringInfo
    *profile;

  if (length < 4)
    return(MagickFalse);
  profile=BlobToStringInfo(datum,(size_t) length);
  if (profile == (StringInfo *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  status=SetImageProfile(image,name,profile,exception);
  profile=DestroyStringInfo(profile);
  if (status == MagickFalse)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  return(MagickTrue);
}