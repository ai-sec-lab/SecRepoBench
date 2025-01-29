  string_info->length=length;
  if (string_info->datum == (unsigned char *) NULL)
    string_info->datum=(unsigned char *) AcquireQuantumMemory(length+
      MagickPathExtent,sizeof(*string_info->datum));
  else
    string_info->datum=(unsigned char *) ResizeQuantumMemory(
      string_info->datum,OverAllocateMemory(length+MagickPathExtent),
      sizeof(*string_info->datum));
  if (string_info->datum == (unsigned char *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) memcpy(string_info->datum+length,source->datum,source->length);