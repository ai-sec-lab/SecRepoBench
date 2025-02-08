static int UnpackWPGRaster(Image *image,int bpp,ExceptionInfo *exception)
{
  int
    x,
    rowIndex,
    i;

  unsigned char
    bbuf,
    *BImgBuff,
    RunCount;

  ssize_t
    ldblk;

  x=0;
  rowIndex=0;

  ldblk=(ssize_t) ((bpp*image->columns+7)/8);
  BImgBuff=(unsigned char *) AcquireQuantumMemory((size_t) ldblk,
    8*sizeof(*BImgBuff));
  if(BImgBuff==NULL) return(-2);
  (void) memset(BImgBuff,0,(size_t) ldblk*8*sizeof(*BImgBuff));
  while (rowIndex < (ssize_t) image->rows)
  {
      int
        c;

      c=ReadBlobByte(image);
      if (c == EOF)
        break;
      bbuf=(unsigned char) c;
      RunCount=bbuf & 0x7F;
      if(bbuf & 0x80)
        {
          if(RunCount)  /* repeat next byte runcount * */
            {
              bbuf=ReadBlobByte(image);
              for(i=0;i<(int) RunCount;i++) InsertByte(bbuf);
            }
          else {  /* read next byte as RunCount; repeat 0xFF runcount* */
            c=ReadBlobByte(image);
            if (c < 0)
              break;
            RunCount=(unsigned char) c;
            for(i=0;i<(int) RunCount;i++) InsertByte(0xFF);
          }
        }
      else {
        if(RunCount)   /* next runcount byte are read directly */
          {
            for(i=0;i < (int) RunCount;i++)
              {
                c=ReadBlobByte(image);
                if (c < 0)
                  break;
                InsertByte(c);
              }
          }
        else {  // <MASK>
        }
      }
      if (EOFBlob(image) != MagickFalse)
        break;
    }
  BImgBuff=(unsigned char *) RelinquishMagickMemory(BImgBuff);
  return(rowIndex <(ssize_t) image->rows ? -5 : 0);
}