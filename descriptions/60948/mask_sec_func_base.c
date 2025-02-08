static MagickBooleanType InsertRow(Image *image,unsigned char *p,ssize_t y,
  int bpp,ExceptionInfo *exception)
{
  int
    bit;

  Quantum
    index,
    *q;

  ssize_t
    x;

  q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
  if (q == (Quantum *) NULL)
    return(MagickFalse);
  switch (bpp)
    {
    // <MASK>
    }
  if (!SyncAuthenticPixels(image,exception))
    return(MagickFalse);
  return(MagickTrue);
}