  ssize_t
    y;

  size_t
    height,
    n,
    width;

  unsigned short
    *pixels;

  /*
    Allocate and initialize histogram arrays.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  colorspace=image->colorspace;
  if (TransformImageColorspace(image,LabColorspace,exception) == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  tile.x=(ssize_t) (x_tiles < 2 ? 2 : x_tiles >= MaxCLAHETiles ?
    MaxCLAHETiles-1 : x_tiles);
  tile.y=(ssize_t) (y_tiles < 2 ? 2 : y_tiles >= MaxCLAHETiles ?
    MaxCLAHETiles-1 : y_tiles);
  width=((image->columns+tile.x/2)/tile.x)*tile.x;
  height=((image->rows+tile.y/2)/tile.y)*tile.y;
  pixel_cache=AcquireVirtualMemory(width,height*sizeof(*pixels));
  if (pixel_cache == (MemoryInfo *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);