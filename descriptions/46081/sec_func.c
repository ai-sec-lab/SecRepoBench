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
  quantum_info->depth=MagickMin(depth,64);
  if (quantum_info->format == FloatingPointQuantumFormat)
    {
      if (quantum_info->depth > 32)
        quantum_info->depth=64;
      else
        if (quantum_info->depth > 24)
          quantum_info->depth=32;
        else
          if (quantum_info->depth > 16)
            quantum_info->depth=24;
          else
            quantum_info->depth=16;
    }
  quantum=(GetPixelChannels(image)+quantum_info->pad)*
    ((quantum_info->depth+7)/8);
  extent=MagickMax(image->columns,image->rows)*quantum*sizeof(double);
  if ((MagickMax(image->columns,image->rows) != 0) &&
      (quantum != (extent/MagickMax(image->columns,image->rows))))
    return(MagickFalse);
  if (quantum_info->pixels != (MemoryInfo **) NULL)
    {
      if (extent <= quantum_info->extent)
        return(MagickTrue);
      DestroyQuantumPixels(quantum_info);
    }
  return(AcquireQuantumPixels(quantum_info,extent));
}