MagickExport MagickBooleanType SetQuantumDepth(const Image *image,
  QuantumInfo *quantumdata,const size_t depth)
{
  size_t
    extent,
    quantum;

  /*
    Allocate the quantum pixel buffer.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(quantumdata != (QuantumInfo *) NULL);
  assert(quantumdata->signature == MagickCoreSignature);
  quantumdata->depth=MagickMin(depth,64);
  // Adjust the quantum depth based on the format and specified depth,
  // particularly for floating point formats. Calculate the memory
  // required for quantum pixels based on the image's channels, padding,
  // and depth. Determine the necessary buffer size by comparing it to
  // the product of the image's dimensions and memory requirement per
  // pixel. Ensure memory allocation for quantum pixels is sufficient;
  // if not, reallocate as needed. Return whether the allocation was
  // successful.
  // <MASK>
}