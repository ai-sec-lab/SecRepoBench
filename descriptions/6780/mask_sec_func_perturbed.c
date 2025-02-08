static int UnpackWPG2Raster(Image *img,int bpp,ExceptionInfo *exception)
{
  int
    RunCount,
    XorMe = 0;

  size_t
    x,
    y;

  ssize_t
    i,
    ldblk;

  unsigned int
    SampleSize=1;

  unsigned char
    bbuf,
    *BImgBuff,
    SampleBuffer[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

  x=0;
  y=0;
  ldblk=(ssize_t) ((bpp*img->columns+7)/8);
  // <MASK>
  BImgBuff=(unsigned char *) RelinquishMagickMemory(BImgBuff);
  return(0);
}