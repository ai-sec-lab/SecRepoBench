static MagickBooleanType GetXMPProperty(const Image *image,const char *property)
{
  char
    *xmp_profile;

  const char
    *content;

  const StringInfo
    *profile;

  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  register const char
    *p;

  XMLTreeInfo
    *child,
    *description,
    *node,
    *rdf,
    *xmp;

  profile=GetImageProfile(image,"xmp");
  if (profile == (StringInfo *) NULL)
    return(MagickFalse);
  // <MASK>
}