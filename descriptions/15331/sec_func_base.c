static MagickBooleanType ReadRectangle(Image *image,PICTRectangle *rectangle)
{
  rectangle->top=(short) ReadBlobMSBShort(image);
  rectangle->left=(short) ReadBlobMSBShort(image);
  rectangle->bottom=(short) ReadBlobMSBShort(image);
  rectangle->right=(short) ReadBlobMSBShort(image);
  if (((EOFBlob(image) != MagickFalse) ||
      (((rectangle->bottom | rectangle->top |
         rectangle->right | rectangle->left ) & 0x8000) != 0) ||
      (rectangle->bottom <= rectangle->top) ||
      (rectangle->right <= rectangle->left)))
    return(MagickFalse);
  return(MagickTrue);
}