if (scanline == (unsigned char *) NULL)
    {
      pixels=(unsigned char *) RelinquishMagickMemory(pixels);
      return((unsigned char *) NULL);
    }
  (void) memset(scanline,0,2*row_bytes*sizeof(*scanline));
  status=MagickTrue;
  if (bytes_per_line < 8)
    {
      /*
        Pixels are already uncompressed.
      */
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        q=pixels+y*width*GetPixelChannels(image);
        number_pixels=bytes_per_line;
        count=ReadBlob(blob,(size_t) number_pixels,scanline);
        if (count != (ssize_t) number_pixels)
          {
            status=MagickFalse;
            break;
          }
        p=UnpackScanline(scanline,bits_per_pixel,unpack_buffer,&number_pixels);
        if ((q+number_pixels) > (pixels+(*extent)))
          {
            status=MagickFalse;
            break;
          }
        (void) memcpy(q,p,(size_t) number_pixels);
      }
      scanline=(unsigned char *) RelinquishMagickMemory(scanline);
      if (status == MagickFalse)
        pixels=(unsigned char *) RelinquishMagickMemory(pixels);
      return(pixels);
    }