unit=1;
  return((MagickSizeType) ((unit << (MagickMin(depth,64)-1))+
    ((unit << (MagickMin(depth,64)-1))-1)));