if (quantumdetails->pixels != (MemoryInfo **) NULL)
    DestroyQuantumPixels(quantumdetails);
  quantum=(quantumdetails->pad+MaxPixelChannels)*(quantumdetails->depth+7)/8;
  extent=MagickMax(image->columns,image->rows)*quantum;
  if ((MagickMax(image->columns,image->rows) != 0) &&
      (quantum != (extent/MagickMax(image->columns,image->rows))))
    return(MagickFalse);