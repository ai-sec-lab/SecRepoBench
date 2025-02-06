case ReadStripMethod:
      {
        register unsigned char
          *p;

        size_t
          extent;

        ssize_t
          stride,
          strip_id;

        tsize_t
          strip_size;

        unsigned char
          *strip_pixels;

        /*
          Convert stripped TIFF image.
        */
        extent=TIFFStripSize(tiff)+sizeof(uint32);
        strip_pixels=(unsigned char *) AcquireQuantumMemory(extent,
          sizeof(*strip_pixels));
        if (strip_pixels == (unsigned char *) NULL)
          ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
        (void) memset(strip_pixels,0,extent*sizeof(*strip_pixels));
        stride=TIFFVStripSize(tiff,1);
        strip_id=0;
        p=strip_pixels;
        for (i=0; i < (ssize_t) samples_per_pixel; i++)
        {
          size_t
            rows_remaining;

          switch (i)
          {
            case 0: break;
            case 1: quantum_type=GreenQuantum; break;
            case 2: quantum_type=BlueQuantum; break;
            case 3:
            {
              if (image->colorspace == CMYKColorspace)
                quantum_type=BlackQuantum;
              break;
            }
            case 4: quantum_type=AlphaQuantum; break;
          }
          rows_remaining=0;
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            register Quantum
              *magick_restrict q;

            q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            if (rows_remaining == 0)
              {
                strip_size=TIFFReadEncodedStrip(tiff,strip_id,strip_pixels,
                  TIFFStripSize(tiff));
                if (strip_size == -1)
                  break;
                rows_remaining=rows_per_strip;
                if ((y+rows_per_strip) > image->rows)
                  rows_remaining=(rows_per_strip-(y+rows_per_strip-
                    image->rows));
                p=strip_pixels;
                strip_id++;
              }
            (void) ImportQuantumPixels(image,(CacheView *) NULL,
              quantum_info,quantum_type,p,exception);
            p+=stride;
            rows_remaining--;
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
          if ((samples_per_pixel > 1) && (interlace != PLANARCONFIG_SEPARATE))
            break;
        }
        strip_pixels=(unsigned char *) RelinquishMagickMemory(strip_pixels);
        break;
      }
      case ReadTileMethod:
      {
        register unsigned char
          *p;

        size_t
          extent;

        uint32
          columns,
          rows;

        unsigned char
          *tile_pixels;

        /*
          Convert tiled TIFF image.
        */
        if ((TIFFGetField(tiff,TIFFTAG_TILEWIDTH,&columns) != 1) ||
            (TIFFGetField(tiff,TIFFTAG_TILELENGTH,&rows) != 1))
          ThrowTIFFException(CoderError,"ImageIsNotTiled");
        if ((AcquireMagickResource(WidthResource,columns) == MagickFalse) ||
            (AcquireMagickResource(HeightResource,rows) == MagickFalse))
          ThrowTIFFException(ImageError,"WidthOrHeightExceedsLimit");
        number_pixels=(MagickSizeType) columns*rows;
        if (HeapOverflowSanityCheck(rows,sizeof(*tile_pixels)) != MagickFalse)
          ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
        extent=TIFFTileSize(tiff)+sizeof(uint32);
        tile_pixels=(unsigned char *) AcquireQuantumMemory(extent,
          sizeof(*tile_pixels));
        if (tile_pixels == (unsigned char *) NULL)
          ThrowTIFFException(ResourceLimitError,"MemoryAllocationFailed");
        (void) memset(tile_pixels,0,TIFFTileSize(tiff)*sizeof(*tile_pixels));
        for (i=0; i < (ssize_t) samples_per_pixel; i++)
        {
          switch (i)
          {
            case 0: break;
            case 1: quantum_type=GreenQuantum; break;
            case 2: quantum_type=BlueQuantum; break;
            case 3:
            {
              if (image->colorspace == CMYKColorspace)
                quantum_type=BlackQuantum;
              break;
            }
            case 4: quantum_type=AlphaQuantum; break;
          }
          for (y=0; y < (ssize_t) image->rows; y+=rows)
          {
            register ssize_t
              x;

            size_t
              rows_remaining;

            rows_remaining=image->rows-y;
            if ((ssize_t) (y+rows) < (ssize_t) image->rows)
              rows_remaining=rows;
            for (x=0; x < (ssize_t) image->columns; x+=columns)
            {
              size_t
                columns_remaining,
                row;

              columns_remaining=image->columns-x;
              if ((ssize_t) (x+columns) < (ssize_t) image->columns)
                columns_remaining=columns;
              if (TIFFReadTile(tiff,tile_pixels,(uint32) x,(uint32) y,0,i) == 0)
                break;
              p=tile_pixels;
              for (row=0; row < rows_remaining; row++)
              {
                register Quantum
                  *magick_restrict q;

                q=GetAuthenticPixels(image,x,y+row,columns_remaining,1,
                  exception);
                if (q == (Quantum *) NULL)
                  break;
                (void) ImportQuantumPixels(image,(CacheView *) NULL,
                  quantum_info,quantum_type,p,exception);
                p+=TIFFTileRowSize(tiff);
                if (SyncAuthenticPixels(image,exception) == MagickFalse)
                  break;
              }
            }
            if (image->previous == (Image *) NULL)
              {
                status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
                  image->rows);
                if (status == MagickFalse)
                  break;
              }
          }
          if ((samples_per_pixel > 1) && (interlace != PLANARCONFIG_SEPARATE))
            break;
        }
        tile_pixels=(unsigned char *) RelinquishMagickMemory(tile_pixels);
        break;
      }