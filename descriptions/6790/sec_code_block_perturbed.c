if (depth == 0)
    return(0);
  unit=1;
  return((MagickSizeType) ((unit << (MagickMin(depth,64)-1))+
    ((unit << (MagickMin(depth,64)-1))-1)));