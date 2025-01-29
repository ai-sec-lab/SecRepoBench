if ((EOFBlob(img) != MagickFalse) || 
      ((rectangle->bottom-rectangle->top) <= 0) ||
      ((rectangle->right-rectangle->left) <= 0))
    return(MagickFalse);
  return(MagickTrue);