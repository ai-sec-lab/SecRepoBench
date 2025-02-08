if (((EOFBlob(image) != MagickFalse) ||
      (((rectangle->bottom | rectangle->top |
         rectangle->right | rectangle->left ) & 0x8000) != 0) ||
      (rectangle->bottom <= rectangle->top) ||
      (rectangle->right <= rectangle->left)))
    return(MagickFalse);
  return(MagickTrue);