if (quantum_info->pixels != (MemoryInfo **) NULL)
    DestroyQuantumPixels(quantum_info);
  quantum=(quantum_info->pad+MaxPixelChannels)*(quantum_info->depth+7)/8;
  extent=4*MagickMax(image->columns,image->rows)*quantum;
  if ((MagickMax(image->columns,image->rows) != 0) &&
      (quantum != (extent/MagickMax(image->columns,image->rows))))
    return(MagickFalse);