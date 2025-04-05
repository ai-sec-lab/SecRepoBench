if (image->type != GrayscaleType)
    (void) TransformImageColorspace(image,GRAYColorspace,exception);
  c_mp_i=(ssize_t *) AcquireQuantumMemory(MaxColormapSize,
    sizeof(*c_mp_i));
  if (c_mp_i == (ssize_t *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);