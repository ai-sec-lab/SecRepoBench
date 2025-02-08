MagickExport MagickBooleanType SetQuantumDepth(const Image *image,
  QuantumInfo *quantum_info,const size_t depth)
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
  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickCoreSignature);
  quantum_info->depth=depth;
  if (quantum_info->format == FloatingPointQuantumFormat)
    {
      if (quantum_info->depth > 32)
        quantum_info->depth=64;
      else
        if (quantum_info->depth > 16)
          quantum_info->depth=32;
        else
          quantum_info->depth=16;
    }
  // <MASK>
  return(AcquireQuantumPixels(quantum_info,extent));
}