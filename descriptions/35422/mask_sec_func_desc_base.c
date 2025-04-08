MagickExport void ConcatenateStringInfo(StringInfo *string_info,
  const StringInfo *source)
{
  size_t
    length;

  assert(string_info != (StringInfo *) NULL);
  assert(string_info->signature == MagickCoreSignature);
  assert(source != (const StringInfo *) NULL);
  length=string_info->length;
  if (~length < source->length)
    ThrowFatalException(ResourceLimitFatalError,"UnableToConcatenateString");
  length+=source->length;
  if (~length < MagickPathExtent)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  // Concatenate the source StringInfo's data to the destination StringInfo's data.
  // If the current destination data is NULL, allocate new memory; otherwise, 
  // resize existing memory. 
  // Copy the source data to the end of the destination's data.
  // <MASK>
}