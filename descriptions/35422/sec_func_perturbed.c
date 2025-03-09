MagickExport void ConcatenateStringInfo(StringInfo *infostring,
  const StringInfo *source)
{
  size_t
    length;

  assert(infostring != (StringInfo *) NULL);
  assert(infostring->signature == MagickCoreSignature);
  assert(source != (const StringInfo *) NULL);
  length=infostring->length;
  if (~length < source->length)
    ThrowFatalException(ResourceLimitFatalError,"UnableToConcatenateString");
  length+=source->length;
  if (~length < MagickPathExtent)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  if (infostring->datum == (unsigned char *) NULL)
    infostring->datum=(unsigned char *) AcquireQuantumMemory(length+
      MagickPathExtent,sizeof(*infostring->datum));
  else
    infostring->datum=(unsigned char *) ResizeQuantumMemory(
      infostring->datum,OverAllocateMemory(length+MagickPathExtent),
      sizeof(*infostring->datum));
  if (infostring->datum == (unsigned char *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) memcpy(infostring->datum+infostring->length,source->datum,source->length);
  infostring->length=length;
}