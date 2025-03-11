static const char *GetICCProperty(const Image *image,const char *key,
  ExceptionInfo *exception_info)
{
  // Retrieve ICC profile properties from an image. First, attempt to get the
  // ICC profile from the image using "icc" or "icm" profile keys. If found,
  // ensure the profile length is adequate for processing. If LCMS is defined,
  // open the ICC profile and extract profile information such as description,
  // manufacturer, model, and copyright. Store these as properties in the image.
  // <MASK>
}