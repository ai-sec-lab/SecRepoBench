quantum=(GetPixelChannels(image)+quantumdata->pad)*
    ((quantumdata->depth+7)/8);
  extent=MagickMax(image->columns,image->rows)*quantum*sizeof(double);