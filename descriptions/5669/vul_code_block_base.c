if (scanline == (unsigned char *) NULL)
    {
      pixels=(unsigned char *) RelinquishMagickMemory(pixels);
      return((unsigned char *) NULL);
    }
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