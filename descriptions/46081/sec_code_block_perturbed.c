quantum=(GetPixelChannels(image)+quantumdata->pad)*
    ((quantumdata->depth+7)/8);
  extent=MagickMax(image->columns,image->rows)*quantum*sizeof(double);
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