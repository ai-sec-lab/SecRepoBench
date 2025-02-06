if (image->type != GrayscaleType)
    (void) TransformImageColorspace(image,GRAYColorspace,exception);
  colormap_index=(ssize_t *) AcquireQuantumMemory(MaxColormapSize,
    sizeof(*colormap_index));