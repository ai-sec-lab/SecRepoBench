quantum=(GetPixelChannels(image)+quantum_info->pad)*
    ((quantum_info->depth+7)/8)*sizeof(double);
  extent=MagickMax(image->columns,image->rows)*quantum;