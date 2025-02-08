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
  if (quantumdata->format == FloatingPointQuantumFormat)
    {
      if (quantumdata->depth > 32)
        quantumdata->depth=64;
      else
        if (quantumdata->depth > 24)
          quantumdata->depth=32;
        else
          if (quantumdata->depth > 16)
            quantumdata->depth=24;
          else
            quantumdata->depth=16;
    }
  // <MASK>
  if ((MagickMax(image->columns,image->rows) != 0) &&
      (quantum != (extent/MagickMax(image->columns,image->rows))))
    return(MagickFalse);
  if (quantumdata->pixels != (MemoryInfo **) NULL)
    {
      if (extent <= quantumdata->extent)
        return(MagickTrue);
      DestroyQuantumPixels(quantumdata);
    }
  return(AcquireQuantumPixels(quantumdata,extent));
}