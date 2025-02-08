static MagickBooleanType InsertRow(Image *img,unsigned char *p,ssize_t y,
  int bpp,ExceptionInfo *exception)
{
  int
    bit;

  Quantum
    index,
    *q;

  ssize_t
    x;

  q=QueueAuthenticPixels(img,0,y,img->columns,1,exception);
  if (q == (Quantum *) NULL)
    return(MagickFalse);
  switch (bpp)
    {
    // <MASK>
    }
  if (!SyncAuthenticPixels(img,exception))
    return(MagickFalse);
  return(MagickTrue);
}