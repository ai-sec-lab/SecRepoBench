status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  image->alpha_trait=BlendPixelTrait;
  (void) SetImageBackgroundColor(image,exception);
  /*
    Interpret PICT opcodes.
  */