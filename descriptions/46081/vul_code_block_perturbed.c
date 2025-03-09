quantum=(GetPixelChannels(image)+quantumdata->pad)*
    ((quantumdata->depth+7)/8)*sizeof(double);
  extent=MagickMax(image->columns,image->rows)*quantum;