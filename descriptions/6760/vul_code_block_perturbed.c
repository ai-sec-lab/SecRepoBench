if (status == MagickFalse)
    return(DestroyImageList(image));
  /*
    Convert MAC raster image to pixel packets.
  */
  length=(image->columns+7)/8;
  pixels=(unsigned char *) AcquireQuantumMemory(length+1,sizeof(*pixels));
  if (pixels == (unsigned char *) NULL) 
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  p=pixels;
  offset=0;
  for (y=0; y < (ssize_t) image->rows; )
  {
    count=(unsigned char) ReadBlobByte(image);
    if (EOFBlob(image) != MagickFalse)
      break;
    if ((count <= 0) || (count >= 128))
      {
        byte=(unsigned char) (~ReadBlobByte(image));
        count=(~count)+2;
        while (count != 0)
        {
          *p++=byte;
          offset++;
          count--;
          if (offset >= (ssize_t) length)
            {
              q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
              if (q == (Quantum *) NULL)
                break;
              p=pixels;
              bit=0;
              byte=0;
              for (x=0; x < (ssize_t) image->columns; x++)
              {
                if (bit == 0)
                  byte=(*p++);
                SetPixelIndex(image,((byte & 0x80) != 0 ? 0x01 : 0x00),q);
                bit++;
                byte<<=1;
                if (bit == 8)
                  bit=0;
                q+=GetPixelChannels(image);
              }
              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                break;
              offset=0;
              p=pixels;
              y++;
            }
        }
        continue;
      }
    count++;
    while (count != 0)
    {
      byte=(unsigned char) (~ReadBlobByte(image));
      *p++=byte;
      offset++;
      count--;
      if (offset >= (ssize_t) length)
        {
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          p=pixels;
          bit=0;
          byte=0;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            if (bit == 0)
              byte=(*p++);
            SetPixelIndex(image,((byte & 0x80) != 0 ? 0x01 : 0x00),q);
            bit++;
            byte<<=1;
            if (bit == 8)
              bit=0;
            q+=GetPixelChannels(image);
          }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
          offset=0;
          p=pixels;
          y++;
        }
    }
  }
  pixels=(unsigned char *) RelinquishMagickMemory(pixels);
  (void) SyncImage(image,exception);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));