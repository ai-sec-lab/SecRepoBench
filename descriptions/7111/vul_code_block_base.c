tiff_pixels=(unsigned char *) AcquireMagickMemory(MagickMax(
      TIFFScanlineSize(tiff),MagickMax((ssize_t) image->columns*
      samples_per_pixel*pow(2.0,ceil(log(bits_per_sample)/log(2.0))),
      rows_per_strip)*sizeof(uint32)));
    if (tiff_pixels == (unsigned char *) NULL)
      ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
    switch (method)
    {
      case ReadSingleSampleMethod:
      {
        /*
          Convert TIFF image to PseudoClass MIFF image.
        */
        quantum_type=IndexQuantum;
        pad=(size_t) MagickMax((ssize_t) samples_per_pixel-1,0);
        if (image->alpha_trait != UndefinedPixelTrait)
          {
            if (image->storage_class != PseudoClass)
              {
                quantum_type=samples_per_pixel == 1 ? AlphaQuantum :
                  GrayAlphaQuantum;
                pad=(size_t) MagickMax((ssize_t) samples_per_pixel-2,0);
              }
            else
              {
                quantum_type=IndexAlphaQuantum;
                pad=(size_t) MagickMax((ssize_t) samples_per_pixel-2,0);
              }
          }
        else
          if (image->storage_class != PseudoClass)
            {
              quantum_type=GrayQuantum;
              pad=(size_t) MagickMax((ssize_t) samples_per_pixel-1,0);
            }
        status=SetQuantumPad(image,quantum_info,pad*pow(2,ceil(log(
          bits_per_sample)/log(2))));
        if (status == MagickFalse)
          ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          int
            status;

          register Quantum
            *magick_restrict q;

          status=TIFFReadPixels(tiff,bits_per_sample,0,y,(char *) tiff_pixels);
          if (status == -1)
            break;
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,tiff_pixels,exception);
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
      case ReadRGBAMethod:
      {
        /*
          Convert TIFF image to DirectClass MIFF image.
        */
        pad=(size_t) MagickMax((size_t) samples_per_pixel-3,0);
        quantum_type=RGBQuantum;
        if (image->alpha_trait != UndefinedPixelTrait)
          {
            quantum_type=RGBAQuantum;
            pad=(size_t) MagickMax((size_t) samples_per_pixel-4,0);
          }
        if (image->colorspace == CMYKColorspace)
          {
            pad=(size_t) MagickMax((size_t) samples_per_pixel-4,0);
            quantum_type=CMYKQuantum;
            if (image->alpha_trait != UndefinedPixelTrait)
              {
                quantum_type=CMYKAQuantum;
                pad=(size_t) MagickMax((size_t) samples_per_pixel-5,0);
              }
          }
        status=SetQuantumPad(image,quantum_info,pad*((bits_per_sample+7) >> 3));
        if (status == MagickFalse)
          ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          int
            status;

          register Quantum
            *magick_restrict q;

          status=TIFFReadPixels(tiff,bits_per_sample,0,y,(char *) tiff_pixels);
          if (status == -1)
            break;
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
            quantum_type,tiff_pixels,exception);
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
      case ReadCMYKAMethod:
      {
        /*
          Convert TIFF image to DirectClass MIFF image.
        */
        for (i=0; i < (ssize_t) samples_per_pixel; i++)
        {
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            register Quantum
              *magick_restrict q;

            int
              status;

            status=TIFFReadPixels(tiff,bits_per_sample,(tsample_t) i,y,(char *)
              tiff_pixels);
            if (status == -1)
              break;
            q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            if (image->colorspace != CMYKColorspace)
              switch (i)
              {
                case 0: quantum_type=RedQuantum; break;
                case 1: quantum_type=GreenQuantum; break;
                case 2: quantum_type=BlueQuantum; break;
                case 3: quantum_type=AlphaQuantum; break;
                default: quantum_type=UndefinedQuantum; break;
              }
            else
              switch (i)
              {
                case 0: quantum_type=CyanQuantum; break;
                case 1: quantum_type=MagentaQuantum; break;
                case 2: quantum_type=YellowQuantum; break;
                case 3: quantum_type=BlackQuantum; break;
                case 4: quantum_type=AlphaQuantum; break;
                default: quantum_type=UndefinedQuantum; break;
              }
            (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
              quantum_type,tiff_pixels,exception);
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
          }
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
      case ReadYCCKMethod:
      {
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          int
            status;

          register Quantum
            *magick_restrict q;

          register ssize_t
            x;

          unsigned char
            *p;

          status=TIFFReadPixels(tiff,bits_per_sample,0,y,(char *) tiff_pixels);
          if (status == -1)
            break;
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          p=tiff_pixels;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            SetPixelCyan(image,ScaleCharToQuantum(ClampYCC((double) *p+
              (1.402*(double) *(p+2))-179.456)),q);
            SetPixelMagenta(image,ScaleCharToQuantum(ClampYCC((double) *p-
              (0.34414*(double) *(p+1))-(0.71414*(double ) *(p+2))+
              135.45984)),q);
            SetPixelYellow(image,ScaleCharToQuantum(ClampYCC((double) *p+
              (1.772*(double) *(p+1))-226.816)),q);
            SetPixelBlack(image,ScaleCharToQuantum((unsigned char) *(p+3)),q);
            q+=GetPixelChannels(image);
            p+=4;
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
      case ReadStripMethod:
      {
        register uint32
          *p;

        /*
          Convert stripped TIFF image to DirectClass MIFF image.
        */
        i=0;
        p=(uint32 *) NULL;
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          register ssize_t
            x;

          register Quantum
            *magick_restrict q;

          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          if (i == 0)
            {
              if (TIFFReadRGBAStrip(tiff,(tstrip_t) y,(uint32 *) tiff_pixels) == 0)
                break;
              i=(ssize_t) MagickMin((ssize_t) rows_per_strip,(ssize_t)
                image->rows-y);
            }
          i--;
          p=((uint32 *) tiff_pixels)+image->columns*i;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            SetPixelRed(image,ScaleCharToQuantum((unsigned char)
              (TIFFGetR(*p))),q);
            SetPixelGreen(image,ScaleCharToQuantum((unsigned char)
              (TIFFGetG(*p))),q);
            SetPixelBlue(image,ScaleCharToQuantum((unsigned char)
              (TIFFGetB(*p))),q);
            if (image->alpha_trait != UndefinedPixelTrait)
              SetPixelAlpha(image,ScaleCharToQuantum((unsigned char)
                (TIFFGetA(*p))),q);
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
      case ReadTileMethod:
      {
        register uint32
          *p;

        uint32
          *tile_pixels,
          columns,
          rows;

        /*
          Convert tiled TIFF image to DirectClass MIFF image.
        */
        if ((TIFFGetField(tiff,TIFFTAG_TILEWIDTH,&columns) != 1) ||
            (TIFFGetField(tiff,TIFFTAG_TILELENGTH,&rows) != 1))
          ThrowTIFFException(CoderError,"ImageIsNotTiled");
        if ((AcquireMagickResource(WidthResource,columns) == MagickFalse) ||
            (AcquireMagickResource(HeightResource,rows) == MagickFalse))
          ThrowTIFFException(ImageError,"WidthOrHeightExceedsLimit");
        (void) SetImageStorageClass(image,DirectClass,exception);
        number_pixels=(MagickSizeType) columns*rows;
        if (HeapOverflowSanityCheck(rows,sizeof(*tile_pixels)) != MagickFalse)
          ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
        tile_pixels=(uint32 *) AcquireQuantumMemory(columns,rows*
          sizeof(*tile_pixels));
        if (tile_pixels == (uint32 *) NULL)
          ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
        for (y=0; y < (ssize_t) image->rows; y+=rows)
        {
          register ssize_t
            x;

          register Quantum
            *magick_restrict q,
            *magick_restrict tile;

          size_t
            columns_remaining,
            rows_remaining;

          rows_remaining=image->rows-y;
          if ((ssize_t) (y+rows) < (ssize_t) image->rows)
            rows_remaining=rows;
          tile=QueueAuthenticPixels(image,0,y,image->columns,rows_remaining,
            exception);
          if (tile == (Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x+=columns)
          {
            size_t
              column,
              row;

            if (TIFFReadRGBATile(tiff,(uint32) x,(uint32) y,tile_pixels) == 0)
              break;
            columns_remaining=image->columns-x;
            if ((ssize_t) (x+columns) < (ssize_t) image->columns)
              columns_remaining=columns;
            p=tile_pixels+(rows-rows_remaining)*columns;
            q=tile+GetPixelChannels(image)*(image->columns*(rows_remaining-1)+
              x);
            for (row=rows_remaining; row > 0; row--)
            {
              if (image->alpha_trait != UndefinedPixelTrait)
                for (column=columns_remaining; column > 0; column--)
                {
                  SetPixelRed(image,ScaleCharToQuantum((unsigned char)
                    TIFFGetR(*p)),q);
                  SetPixelGreen(image,ScaleCharToQuantum((unsigned char)
                    TIFFGetG(*p)),q);
                  SetPixelBlue(image,ScaleCharToQuantum((unsigned char)
                    TIFFGetB(*p)),q);
                  SetPixelAlpha(image,ScaleCharToQuantum((unsigned char)
                    TIFFGetA(*p)),q);
                  p++;
                  q+=GetPixelChannels(image);
                }
              else
                for (column=columns_remaining; column > 0; column--)
                {
                  SetPixelRed(image,ScaleCharToQuantum((unsigned char)
                    TIFFGetR(*p)),q);
                  SetPixelGreen(image,ScaleCharToQuantum((unsigned char)
                    TIFFGetG(*p)),q);
                  SetPixelBlue(image,ScaleCharToQuantum((unsigned char)
                    TIFFGetB(*p)),q);
                  p++;
                  q+=GetPixelChannels(image);
                }
              p+=columns-columns_remaining;
              q-=GetPixelChannels(image)*(image->columns+columns_remaining);
            }
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
        tile_pixels=(uint32 *) RelinquishMagickMemory(tile_pixels);
        break;
      }
      case ReadGenericMethod:
      default:
      {
        MemoryInfo
          *pixel_info;

        register uint32
          *p;

        uint32
          *pixels;

        /*
          Convert TIFF image to DirectClass MIFF image.
        */
        number_pixels=(MagickSizeType) image->columns*image->rows;
        if (HeapOverflowSanityCheck(image->rows,sizeof(*pixels)) != MagickFalse)
          ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
        pixel_info=AcquireVirtualMemory(image->columns,image->rows*
          sizeof(uint32));
        if (pixel_info == (MemoryInfo *) NULL)
          ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
        pixels=(uint32 *) GetVirtualMemoryBlob(pixel_info);
        (void) TIFFReadRGBAImage(tiff,(uint32) image->columns,(uint32)
          image->rows,(uint32 *) pixels,0);
        /*
          Convert image to DirectClass pixel packets.
        */
        p=pixels+number_pixels-1;
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          register ssize_t
            x;

          register Quantum
            *magick_restrict q;

          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          q+=GetPixelChannels(image)*(image->columns-1);
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            SetPixelRed(image,ScaleCharToQuantum((unsigned char)
              TIFFGetR(*p)),q);
            SetPixelGreen(image,ScaleCharToQuantum((unsigned char)
              TIFFGetG(*p)),q);
            SetPixelBlue(image,ScaleCharToQuantum((unsigned char)
              TIFFGetB(*p)),q);
            if (image->alpha_trait != UndefinedPixelTrait)
              SetPixelAlpha(image,ScaleCharToQuantum((unsigned char)
                TIFFGetA(*p)),q);
            p--;
            q-=GetPixelChannels(image);
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
        pixel_info=RelinquishVirtualMemory(pixel_info);
        break;
      }
    }