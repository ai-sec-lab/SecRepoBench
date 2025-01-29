if (depth == 0)
    return(0);
  one=1;
  return((MagickSizeType) ((one << (MagickMin(depth,64)-1))+
    ((one << (MagickMin(depth,64)-1))-1)));