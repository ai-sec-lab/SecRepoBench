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
  // <MASK>
}