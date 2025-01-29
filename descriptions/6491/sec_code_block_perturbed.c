pixels=(unsigned char *) AcquireQuantumMemory(image->columns,4*
    sizeof(*pixels));
  if (pixels == (unsigned char *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(pixels,0,4*image->columns*sizeof(*pixels));
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    if (image->compression != RLECompression)
      {
        count=ReadBlob(image,4*image->columns*sizeof(*pixels),pixels);
        if (count != (ssize_t) (4*image->columns*sizeof(*pixels)))
          break;
      }
    else
      {
        count=ReadBlob(image,4*sizeof(*pixel),pixel);
        if (count != 4)
          break;
        if ((size_t) ((((size_t) pixel[2]) << 8) | pixel[3]) != image->columns)
          {
            (void) memcpy(pixels,pixel,4*sizeof(*pixel));
            count=ReadBlob(image,4*(image->columns-1)*sizeof(*pixels),pixels+4);
            image->compression=NoCompression;
          }
        else
          {
            p=pixels;
            for (i=0; i < 4; i++)
            {
              end=&pixels[(i+1)*image->columns];
              while (p < end)
              {
                count=ReadBlob(image,2*sizeof(*pixel),pixel);
                if (count < 1)
                  break;
                if (pixel[0] > 128)
                  {
                    count=(ssize_t) pixel[0]-128;
                    if ((count == 0) || (count > (ssize_t) (end-p)))
                      break;
                    while (count-- > 0)
                      *p++=pixel[1];
                  }
                else
                  {
                    count=(ssize_t) pixel[0];
                    if ((count == 0) || (count > (ssize_t) (end-p)))
                      break;
                    *p++=pixel[1];
                    if (--count > 0)
                      {
                        count=ReadBlob(image,(size_t) count*sizeof(*p),p);
                        if (count < 1)
                          break;
                        p+=count;
                      }
                  }
              }
            }
          }
      }
    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    i=0;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      if (image->compression == RLECompression)
        {
          pixel[0]=pixels[x];
          pixel[1]=pixels[x+image->columns];
          pixel[2]=pixels[x+2*image->columns];
          pixel[3]=pixels[x+3*image->columns];
        }
      else
        {
          pixel[0]=pixels[i++];
          pixel[1]=pixels[i++];
          pixel[2]=pixels[i++];
          pixel[3]=pixels[i++];
        }
      SetPixelRed(image,0,q);
      SetPixelGreen(image,0,q);
      SetPixelBlue(image,0,q);
      if (pixel[3] != 0)
        {
          gamma=pow(2.0,pixel[3]-(128.0+8.0));
          SetPixelRed(image,ClampToQuantum(QuantumRange*gamma*pixel[0]),q);
          SetPixelGreen(image,ClampToQuantum(QuantumRange*gamma*pixel[1]),q);
          SetPixelBlue(image,ClampToQuantum(QuantumRange*gamma*pixel[2]),q);
        }
      q+=GetPixelChannels(image);
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
    status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }