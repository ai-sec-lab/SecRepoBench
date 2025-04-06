if (image->type != GrayscaleType)
    (void) TransformImageColorspace(image,GRAYColorspace,exception);
  if (image->storage_class == PseudoClass)
    c_mp_i=(ssize_t *) AcquireQuantumMemory(image->colors,
      sizeof(*c_mp_i));
  else
    c_mp_i=(ssize_t *) AcquireQuantumMemory(MaxColormapSize,
      sizeof(*c_mp_i));
  if (c_mp_i == (ssize_t *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);