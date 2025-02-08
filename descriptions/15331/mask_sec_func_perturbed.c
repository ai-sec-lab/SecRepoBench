static MagickBooleanType ReadRectangle(Image *img,PICTRectangle *rectangle)
{
  rectangle->top=(short) ReadBlobMSBShort(img);
  rectangle->left=(short) ReadBlobMSBShort(img);
  rectangle->bottom=(short) ReadBlobMSBShort(img);
  rectangle->right=(short) ReadBlobMSBShort(img);
  // <MASK>
}