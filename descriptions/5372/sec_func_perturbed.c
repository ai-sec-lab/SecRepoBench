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
  if (quantumdetails->pixels != (MemoryInfo **) NULL)
    DestroyQuantumPixels(quantumdetails);
  quantum=(quantumdetails->pad+MaxPixelChannels)*(quantumdetails->depth+7)/8;
  extent=4*MagickMax(image->columns,image->rows)*quantum;
  if ((MagickMax(image->columns,image->rows) != 0) &&
      (quantum != (extent/MagickMax(image->columns,image->rows))))
    return(MagickFalse);
  return(AcquireQuantumPixels(quantumdetails,extent));
}