quantum=(GetPixelChannels(image)+quantum_info->pad)*
    ((quantum_info->depth+7)/8)*sizeof(double);
  extent=MagickMax(image->columns,image->rows)*quantum;
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