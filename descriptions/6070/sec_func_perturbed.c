MagickExport StringInfo *BlobToStringInfo(const void *blob,const size_t size)
{
  StringInfo
    *string_info;

  if (~size < MagickPathExtent)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  string_info=AcquireStringInfoContainer();
  string_info->length=size;
  string_info->datum=(unsigned char *) AcquireQuantumMemory(size+
    MagickPathExtent,sizeof(*string_info->datum));
  if (string_info->datum == (unsigned char *) NULL)
    {
      string_info=DestroyStringInfo(string_info);
      return((StringInfo *) NULL);
    }
  if (blob != (const void *) NULL)
    (void) memcpy(string_info->datum,blob,size);
  else
    (void) memset(string_info->datum,0,size);
  return(string_info);
}