MagickExport StringInfo *BlobToStringInfo(const void *blob,const size_t length)
{
  StringInfo
    *string_info;

  if (~length < MagickPathExtent)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  string_info=AcquireStringInfoContainer();
  string_info->length=length;
  string_info->datum=(unsigned char *) AcquireQuantumMemory(length+
    MagickPathExtent,sizeof(*string_info->datum));
  if (string_info->datum == (unsigned char *) NULL)
    {
      string_info=DestroyStringInfo(string_info);
      return((StringInfo *) NULL);
    }
  if (blob != (const void *) NULL)
    (void) memcpy(string_info->datum,blob,length);
  else
    (void) memset(string_info->datum,0,length);
  return(string_info);
}