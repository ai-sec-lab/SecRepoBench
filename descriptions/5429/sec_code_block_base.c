if (image->type != GrayscaleType)
    (void) TransformImageColorspace(image,GRAYColorspace,exception);
  if (image->storage_class == PseudoClass)
    colormap_index=(ssize_t *) AcquireQuantumMemory(image->colors,
      sizeof(*colormap_index));
  else
    colormap_index=(ssize_t *) AcquireQuantumMemory(MaxColormapSize,
      sizeof(*colormap_index));
  if (colormap_index == (ssize_t *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);