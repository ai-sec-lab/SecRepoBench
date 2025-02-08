MagickExport MagickBooleanType CLAHEImage(Image *image,const size_t x_tiles,
  const size_t y_tiles,const size_t number_bins,const double clip_limit,
  ExceptionInfo *exception)
{
#define CLAHEImageTag  "CLAHE/Image"

  CacheView
    *image_view;

  ColorspaceType
    colorspace;

  MagickBooleanType
    status;

  MagickOffsetType
    completion;

  MemoryInfo
    *pixel_cache;

  OffsetInfo
    tile;

  size_t
    height,
    n,
    width;

  ssize_t
    y;

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
  tile.x=(ssize_t) (x_tiles < 2 ? 2 : x_tiles >= MaxCLAHETiles ? MaxCLAHETiles-
    1 : x_tiles);
  tile.y=(ssize_t) (y_tiles < 2 ? 2 : y_tiles >= MaxCLAHETiles ? MaxCLAHETiles-
    1 : y_tiles);
  width=((image->columns+tile.x/2)/tile.x)*tile.x;
  height=((image->rows+tile.y/2)/tile.y)*tile.y;
  pixel_cache=AcquireVirtualMemory(width,height*sizeof(*pixels));
  if (pixel_cache == (MemoryInfo *) NULL)
    {
      if (TransformImageColorspace(image,colorspace,exception) == MagickFalse)
        return(MagickFalse);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  pixels=(unsigned short *) GetVirtualMemoryBlob(pixel_cache);
  image_view=AcquireVirtualCacheView(image,exception);
  n=0;
  for (y=0; y < (ssize_t) height; y++)
  {
    register const Quantum
      *magick_restrict p;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,width,1,exception);
    if (p == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      pixels[n++]=ScaleQuantumToShort(p[0]);
      p+=GetPixelChannels(image);
    }
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        completion++;
        proceed=SetImageProgress(image,CLAHEImageTag,completion,2*
          GetPixelChannels(image));
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  status=CLAHE(width,height,0,65535,(size_t) tile.x,(size_t) tile.y,
    number_bins == 0 ? (size_t) 128 : MagickMin(number_bins,256),clip_limit,
    pixels);
  if (status == MagickFalse)
    (void) ThrowMagickException(exception,GetMagickModule(),
      ResourceLimitError,"MemoryAllocationFailed","`%s'",image->filename);
  image_view=AcquireAuthenticCacheView(image,exception);
  n=0;
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      q[0]=ScaleShortToQuantum(pixels[n++]);
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        MagickBooleanType
          proceed;

#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp atomic
#endif
        completion++;
        proceed=SetImageProgress(image,CLAHEImageTag,completion,2*
          GetPixelChannels(image));
        if (proceed == MagickFalse)
          status=MagickFalse;
      }
  }
  image_view=DestroyCacheView(image_view);
  pixel_cache=RelinquishVirtualMemory(pixel_cache);
  return(TransformImageColorspace(image,colorspace,exception));
}