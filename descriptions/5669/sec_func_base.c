static unsigned char *DecodeImage(Image *blob,Image *image,
  size_t bytes_per_line,const unsigned int bits_per_pixel,size_t *extent,
  ExceptionInfo *exception)
{
  MagickSizeType
    number_pixels;

  register ssize_t
    i;

  register unsigned char
    *p,
    *q;

  size_t
    bytes_per_pixel,
    length,
    row_bytes,
    scanline_length,
    width;

  ssize_t
    count,
    j,
    y;

  unsigned char
    *pixels,
    *scanline;

  /*
    Determine pixel buffer size.
  */
  if (bits_per_pixel <= 8)
    bytes_per_line&=0x7fff;
  width=image->columns;
  bytes_per_pixel=1;
  if (bits_per_pixel == 16)
    {
      bytes_per_pixel=2;
      width*=2;
    }
  else
    if (bits_per_pixel == 32)
      width*=image->alpha_trait ? 4 : 3;
  if (bytes_per_line == 0)
    bytes_per_line=width;
  row_bytes=(size_t) (image->columns | 0x8000);
  if (image->storage_class == DirectClass)
    row_bytes=(size_t) ((4*image->columns) | 0x8000);
  /*
    Allocate pixel and scanline buffer.
  */
  pixels=(unsigned char *) AcquireQuantumMemory(image->rows,row_bytes*
    sizeof(*pixels));
  if (pixels == (unsigned char *) NULL)
    return((unsigned char *) NULL);
  *extent=row_bytes*image->rows*sizeof(*pixels);
  (void) ResetMagickMemory(pixels,0,*extent);
  scanline=(unsigned char *) AcquireQuantumMemory(row_bytes,2*
    sizeof(*scanline));
  if (scanline == (unsigned char *) NULL)
    {
      pixels=(unsigned char *) RelinquishMagickMemory(pixels);
      return((unsigned char *) NULL);
    }
  (void) ResetMagickMemory(scanline,0,2*row_bytes*sizeof(*scanline));
  if (bytes_per_line < 8)
    {
      /*
        Pixels are already uncompressed.
      */
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        q=pixels+y*width*GetPixelChannels(image);;
        number_pixels=bytes_per_line;
        count=ReadBlob(blob,(size_t) number_pixels,scanline);
        if (count != (ssize_t) number_pixels)
          {
            (void) ThrowMagickException(exception,GetMagickModule(),
              CorruptImageError,"UnableToUncompressImage","`%s'",
              image->filename);
            break;
          }
        p=ExpandBuffer(scanline,&number_pixels,bits_per_pixel);
        if ((q+number_pixels) > (pixels+(*extent)))
          {
            (void) ThrowMagickException(exception,GetMagickModule(),
              CorruptImageError,"UnableToUncompressImage","`%s'",
              image->filename);
            break;
          }
        (void) CopyMagickMemory(q,p,(size_t) number_pixels);
      }
      scanline=(unsigned char *) RelinquishMagickMemory(scanline);
      return(pixels);
    }
  /*
    Uncompress RLE pixels into uncompressed pixel buffer.
  */
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    q=pixels+y*width;
    if (bytes_per_line > 200)
      scanline_length=ReadBlobMSBShort(blob);
    else
      scanline_length=1UL*ReadBlobByte(blob);
    if (scanline_length >= row_bytes)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),
          CorruptImageError,"UnableToUncompressImage","`%s'",image->filename);
        break;
      }
    count=ReadBlob(blob,scanline_length,scanline);
    if (count != (ssize_t) scanline_length)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),
          CorruptImageError,"UnableToUncompressImage","`%s'",image->filename);
        break;
      }
    for (j=0; j < (ssize_t) scanline_length; )
      if ((scanline[j] & 0x80) == 0)
        {
          length=(size_t) ((scanline[j] & 0xff)+1);
          number_pixels=length*bytes_per_pixel;
          p=ExpandBuffer(scanline+j+1,&number_pixels,bits_per_pixel);
          if ((q-pixels+number_pixels) <= *extent)
            (void) CopyMagickMemory(q,p,(size_t) number_pixels);
          q+=number_pixels;
          j+=(ssize_t) (length*bytes_per_pixel+1);
        }
      else
        {
          length=(size_t) (((scanline[j] ^ 0xff) & 0xff)+2);
          number_pixels=bytes_per_pixel;
          p=ExpandBuffer(scanline+j+1,&number_pixels,bits_per_pixel);
          for (i=0; i < (ssize_t) length; i++)
          {
            if ((q-pixels+number_pixels) <= *extent)
              (void) CopyMagickMemory(q,p,(size_t) number_pixels);
            q+=number_pixels;
          }
          j+=(ssize_t) bytes_per_pixel+1;
        }
  }
  scanline=(unsigned char *) RelinquishMagickMemory(scanline);
  return(pixels);
}