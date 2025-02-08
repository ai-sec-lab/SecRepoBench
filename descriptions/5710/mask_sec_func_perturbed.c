static int UnpackWPGRaster(Image *image,int bpp,ExceptionInfo *exception)
{
  int
    x,
    row,
    i;

  unsigned char
    bbuf,
    *BImgBuff,
    RunCount;

  ssize_t
    ldblk;

  x=0;
  row=0;

  ldblk=(ssize_t) ((bpp*image->columns+7)/8);
  // <MASK>
  BImgBuff=(unsigned char *) RelinquishMagickMemory(BImgBuff);
  return(row <(ssize_t) image->rows ? -5 : 0);
}