MagickExport MagickBooleanType SetQuantumDepth(const Image *image,
  QuantumInfo *quantumdetails,const size_t depth)
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
  assert(quantumdetails != (QuantumInfo *) NULL);
  assert(quantumdetails->signature == MagickCoreSignature);
  quantumdetails->depth=depth;
  if (quantumdetails->format == FloatingPointQuantumFormat)
    {
      if (quantumdetails->depth > 32)
        quantumdetails->depth=64;
      else
        if (quantumdetails->depth > 16)
          quantumdetails->depth=32;
        else
          quantumdetails->depth=16;
    }
  // <MASK>
  return(AcquireQuantumPixels(quantumdetails,extent));
}