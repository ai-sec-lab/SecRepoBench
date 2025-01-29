if (status == MagickFalse)
      return(DestroyImageList(image));
    status=ResetImagePixels(image,exception);
    if (status == MagickFalse)
      return(DestroyImageList(image));
    tim_pixels=(unsigned char *) AcquireQuantumMemory(image_size,
      sizeof(*tim_pixels));
    if (tim_pixels == (unsigned char *) NULL)
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    count=ReadBlob(image,image_size,tim_pixels);
    if (count != (ssize_t) (image_size))
      {
        tim_pixels=(unsigned char *) RelinquishMagickMemory(tim_pixels);
        ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
      }
    /*
      Convert TIM raster image to pixel packets.
    */
    switch (bits_per_pixel)
    {
      case 4:
      {
        /*
          Convert PseudoColor scanline.
        */
        for (y=(ssize_t) image->rows-1; y >= 0; y--)
        {
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          p=tim_pixels+y*bytes_per_line;
          for (x=0; x < ((ssize_t) image->columns-1); x+=2)
          {
            SetPixelIndex(image,(*p) & 0x0f,q);
            q+=GetPixelChannels(image);
            SetPixelIndex(image,(*p >> 4) & 0x0f,q);
            p++;
            q+=GetPixelChannels(image);
          }
          if ((image->columns % 2) != 0)
            {
              SetPixelIndex(image,(*p >> 4) & 0x0f,q);
              p++;
              q+=GetPixelChannels(image);
            }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      case 8:
      {
        /*
          Convert PseudoColor scanline.
        */
        for (y=(ssize_t) image->rows-1; y >= 0; y--)
        {
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          p=tim_pixels+y*bytes_per_line;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            SetPixelIndex(image,*p++,q);
            q+=GetPixelChannels(image);
          }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      case 16:
      {
        /*
          Convert DirectColor scanline.
        */
        for (y=(ssize_t) image->rows-1; y >= 0; y--)
        {
          p=tim_pixels+y*bytes_per_line;
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            colorword=(*p++);
            colorword|=(*p++ << 8);
            SetPixelBlue(image,ScaleCharToQuantum(ScaleColor5to8(
              (1UL*colorword >> 10) & 0x1f)),q);
            SetPixelGreen(image,ScaleCharToQuantum(ScaleColor5to8(
              (1UL*colorword >> 5) & 0x1f)),q);
            SetPixelRed(image,ScaleCharToQuantum(ScaleColor5to8(
              (1UL*colorword >> 0) & 0x1f)),q);
            q+=GetPixelChannels(image);
          }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      case 24:
      {
        /*
          Convert DirectColor scanline.
        */
        for (y=(ssize_t) image->rows-1; y >= 0; y--)
        {
          p=tim_pixels+y*bytes_per_line;
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            SetPixelRed(image,ScaleCharToQuantum(*p++),q);
            SetPixelGreen(image,ScaleCharToQuantum(*p++),q);
            SetPixelBlue(image,ScaleCharToQuantum(*p++),q);
            q+=GetPixelChannels(image);
          }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      default:
      {
        tim_pixels=(unsigned char *) RelinquishMagickMemory(tim_pixels);
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
      }
    }
    if (image->storage_class == PseudoClass)
      (void) SyncImage(image,exception);
    tim_pixels=(unsigned char *) RelinquishMagickMemory(tim_pixels);
    if (EOFBlob(image) != MagickFalse)
      {
        ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
          image->filename);
        break;
      }
    /*
      Proceed to next image.
    */
    tim_info.id=ReadBlobLSBLong(image);
    if (tim_info.id == 0x00000010)
      {
        /*
          Allocate next image structure.
        */
        AcquireNextImage(image_info,image,exception);
        if (GetNextImageInList(image) == (Image *) NULL)
          {
            image=DestroyImageList(image);
            return((Image *) NULL);
          }
        image=SyncNextImageInList(image);
        status=SetImageProgress(image,LoadImagesTag,TellBlob(image),
          GetBlobSize(image));
        if (status == MagickFalse)
          break;
      }