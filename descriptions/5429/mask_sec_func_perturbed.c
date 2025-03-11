static MagickBooleanType SetGrayscaleImage(Image *image,
  ExceptionInfo *exception_info)
{
  CacheView
    *image_view;

  MagickBooleanType
    status;

  PixelInfo
    *colormap;

  register ssize_t
    i;

  ssize_t
    *colormap_index,
    j,
    y;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  // <MASK>
  if (image->storage_class != PseudoClass)
    {
      (void) ResetMagickMemory(colormap_index,(-1),MaxColormapSize*
        sizeof(*colormap_index));
      if (AcquireImageColormap(image,MaxColormapSize,exception_info) == MagickFalse)
        {
          colormap_index=(ssize_t *) RelinquishMagickMemory(colormap_index);
          ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
            image->filename);
        }
      image->colors=0;
      status=MagickTrue;
      image_view=AcquireAuthenticCacheView(image,exception_info);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
      #pragma omp parallel for schedule(static,4) shared(status) \
        magick_number_threads(image,image,image->rows,1)
#endif
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        register Quantum
          *magick_restrict q;

        register ssize_t
          x;

        if (status == MagickFalse)
          continue;
        q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,
          exception_info);
        if (q == (Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          register size_t
            intensity;

          intensity=ScaleQuantumToMap(GetPixelRed(image,q));
          if (colormap_index[intensity] < 0)
            {
#if defined(MAGICKCORE_OPENMP_SUPPORT)
              #pragma omp critical (MagickCore_SetGrayscaleImage)
#endif
              if (colormap_index[intensity] < 0)
                {
                  colormap_index[intensity]=(ssize_t) image->colors;
                  image->colormap[image->colors].red=(double)
                    GetPixelRed(image,q);
                  image->colormap[image->colors].green=(double)
                    GetPixelGreen(image,q);
                  image->colormap[image->colors].blue=(double)
                    GetPixelBlue(image,q);
                  image->colors++;
               }
            }
          SetPixelIndex(image,(Quantum) colormap_index[intensity],q);
          q+=GetPixelChannels(image);
        }
        if (SyncCacheViewAuthenticPixels(image_view,exception_info) == MagickFalse)
          status=MagickFalse;
      }
      image_view=DestroyCacheView(image_view);
    }
  for (i=0; i < (ssize_t) image->colors; i++)
    image->colormap[i].alpha=(double) i;
  qsort((void *) image->colormap,image->colors,sizeof(PixelInfo),
    IntensityCompare);
  colormap=(PixelInfo *) AcquireQuantumMemory(image->colors,sizeof(*colormap));
  if (colormap == (PixelInfo *) NULL)
    {
      colormap_index=(ssize_t *) RelinquishMagickMemory(colormap_index);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  j=0;
  colormap[j]=image->colormap[0];
  for (i=0; i < (ssize_t) image->colors; i++)
  {
    if (IsPixelInfoEquivalent(&colormap[j],&image->colormap[i]) == MagickFalse)
      {
        j++;
        colormap[j]=image->colormap[i];
      }
    colormap_index[(ssize_t) image->colormap[i].alpha]=j;
  }
  image->colors=(size_t) (j+1);
  image->colormap=(PixelInfo *) RelinquishMagickMemory(image->colormap);
  image->colormap=colormap;
  status=MagickTrue;
  image_view=AcquireAuthenticCacheView(image,exception_info);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register Quantum
      *magick_restrict q;

    register ssize_t
      x;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception_info);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      SetPixelIndex(image,(Quantum) colormap_index[ScaleQuantumToMap(
        GetPixelIndex(image,q))],q);
      q+=GetPixelChannels(image);
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception_info) == MagickFalse)
      status=MagickFalse;
  }
  image_view=DestroyCacheView(image_view);
  colormap_index=(ssize_t *) RelinquishMagickMemory(colormap_index);
  image->type=GrayscaleType;
  if (SetImageMonochrome(image,exception_info) != MagickFalse)
    image->type=BilevelType;
  return(status);
}