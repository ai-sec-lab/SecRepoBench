one=1;
  return((MagickSizeType) ((one << (MagickMin(depth,64)-1))+
    ((one << (MagickMin(depth,64)-1))-1)));