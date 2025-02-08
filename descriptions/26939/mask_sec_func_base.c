static MagickBooleanType SetImageProfileInternal(Image *image,const char *name,
  const StringInfo *profile,const MagickBooleanType recursive,
  ExceptionInfo *exception)
{
  char
    key[MagickPathExtent];

  MagickBooleanType
    status;

  StringInfo
    *clone_profile;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  clone_profile=CloneStringInfo(profile);
  // <MASK>
}