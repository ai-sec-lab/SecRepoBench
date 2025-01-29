ResetMagickMemory(sun_pixels,0,pixels_length*sizeof(*sun_pixels));
    if (sun_info.type == RT_ENCODED)
      {
        status=DecodeImage(sun_data,sun_info.length,sun_pixels,pixels_length);
        if (status == MagickFalse)
          {
            sun_data=(unsigned char *) RelinquishMagickMemory(sun_data);
            sun_pixels=(unsigned char *) RelinquishMagickMemory(sun_pixels);
            ThrowReaderException(CorruptImageError,"UnableToReadImageData");
          }
      }
    else
      {
        if (sun_info.length > pixels_length)
          {
            sun_data=(unsigned char *) RelinquishMagickMemory(sun_data);
            sun_pixels=(unsigned char *) RelinquishMagickMemory(sun_pixels);
            ThrowReaderException(ResourceLimitError,"ImproperImageHeader");
          }
        (void) CopyMagickMemory(sun_pixels,sun_data,sun_info.length);
      }
    sun_data=(unsigned char *) RelinquishMagickMemory(sun_data);
    /*
      Convert SUN raster image to pixel packets.
    */
    p=sun_pixels;
    if (sun_info.depth == 1)
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < ((ssize_t) image->columns-7); x+=8)
        {
          for (bit=7; bit >= 0; bit--)
          {
            SetPixelIndex(image,(Quantum) ((*p) & (0x01 << bit) ? 0x00 : 0x01),
              q);
            q+=GetPixelChannels(image);
          }
          p++;
        }
        if ((image->columns % 8) != 0)
          {
            for (bit=7; bit >= (int) (8-(image->columns % 8)); bit--)
            {
              SetPixelIndex(image,(Quantum) ((*p) & (0x01 << bit) ? 0x00 :
                0x01),q);
              q+=GetPixelChannels(image);
            }
            p++;
          }
        if ((((image->columns/8)+(image->columns % 8 ? 1 : 0)) % 2) != 0)
          p++;
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
    else
      if (image->storage_class == PseudoClass)
        {
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              SetPixelIndex(image,ConstrainColormapIndex(image,*p,exception),q);
              p++;
              q+=GetPixelChannels(image);
            }
            if ((image->columns % 2) != 0)
              p++;
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
        }
      else
        {
          size_t
            bytes_per_pixel;

          bytes_per_pixel=3;
          if (image->alpha_trait != UndefinedPixelTrait)
            bytes_per_pixel++;
          if (bytes_per_line == 0)
            bytes_per_line=bytes_per_pixel*image->columns;
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              if (image->alpha_trait != UndefinedPixelTrait)
                SetPixelAlpha(image,ScaleCharToQuantum(*p++),q);
              if (sun_info.type == RT_STANDARD)
                {
                  SetPixelBlue(image,ScaleCharToQuantum(*p++),q);
                  SetPixelGreen(image,ScaleCharToQuantum(*p++),q);
                  SetPixelRed(image,ScaleCharToQuantum(*p++),q);
                }
              else
                {
                  SetPixelRed(image,ScaleCharToQuantum(*p++),q);
                  SetPixelGreen(image,ScaleCharToQuantum(*p++),q);
                  SetPixelBlue(image,ScaleCharToQuantum(*p++),q);
                }
              if (image->colors != 0)
                {
                  SetPixelRed(image,ClampToQuantum(image->colormap[(ssize_t)
                    GetPixelRed(image,q)].red),q);
                  SetPixelGreen(image,ClampToQuantum(image->colormap[(ssize_t)
                    GetPixelGreen(image,q)].green),q);
                  SetPixelBlue(image,ClampToQuantum(image->colormap[(ssize_t)
                    GetPixelBlue(image,q)].blue),q);
                }
              q+=GetPixelChannels(image);
            }
            if (((bytes_per_pixel*image->columns) % 2) != 0)
              p++;
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
        }
    if (image->storage_class == PseudoClass)
      (void) SyncImage(image,exception);
    sun_pixels=(unsigned char *) RelinquishMagickMemory(sun_pixels);
    if (EOFBlob(image) != MagickFalse)
      {
        ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
          image->filename);
        break;
      }
    /*
      Proceed to next image.
    */
    if (image_info->number_scenes != 0)
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    sun_info.magic=ReadBlobMSBLong(image);
    if (sun_info.magic == 0x59a66a95)
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