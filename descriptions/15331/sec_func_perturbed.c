static MagickBooleanType ReadRectangle(Image *img,PICTRectangle *rectangle)
{
  rectangle->top=(short) ReadBlobMSBShort(img);
  rectangle->left=(short) ReadBlobMSBShort(img);
  rectangle->bottom=(short) ReadBlobMSBShort(img);
  rectangle->right=(short) ReadBlobMSBShort(img);
  if (((EOFBlob(img) != MagickFalse) ||
      (((rectangle->bottom | rectangle->top |
         rectangle->right | rectangle->left ) & 0x8000) != 0) ||
      (rectangle->bottom <= rectangle->top) ||
      (rectangle->right <= rectangle->left)))
    return(MagickFalse);
  return(MagickTrue);
}