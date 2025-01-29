if ((code == 0x9a) || (code == 0x9b) ||
                ((bytesperrow & 0x8000) != 0))
              {
                if (ReadPixmap(image,&pixmap) == MagickFalse)
                  ThrowPICTException(CorruptImageError,"ImproperImageHeader");
                tile_image->depth=(size_t) pixmap.component_size;
                tile_image->alpha_trait=pixmap.component_count == 4 ?
                  BlendPixelTrait : UndefinedPixelTrait;
                tile_image->resolution.x=(double) pixmap.horizontal_resolution;
                tile_image->resolution.y=(double) pixmap.vertical_resolution;
                tile_image->units=PixelsPerInchResolution;
                if (tile_image->alpha_trait != UndefinedPixelTrait)
                  (void) SetImageAlpha(image,OpaqueAlphaChannel,exception);
              }
            if ((code != 0x9a) && (code != 0x9b))
              {
                /*
                  Initialize colormap.
                */
                tile_image->colors=2;
                if ((bytesperrow & 0x8000) != 0)
                  {
                    (void) ReadBlobMSBLong(image);
                    flags=(ssize_t) ReadBlobMSBShort(image);
                    tile_image->colors=1UL*ReadBlobMSBShort(image)+1;
                  }
                status=AcquireImageColormap(tile_image,tile_image->colors,
                  exception);
                if (status == MagickFalse)
                  ThrowPICTException(ResourceLimitError,
                    "MemoryAllocationFailed");
                if ((bytesperrow & 0x8000) != 0)
                  {
                    for (i=0; i < (ssize_t) tile_image->colors; i++)
                    {
                      j=ReadBlobMSBShort(image) % tile_image->colors;
                      if ((flags & 0x8000) != 0)
                        j=(size_t) i;
                      tile_image->colormap[j].red=(Quantum)
                        ScaleShortToQuantum(ReadBlobMSBShort(image));
                      tile_image->colormap[j].green=(Quantum)
                        ScaleShortToQuantum(ReadBlobMSBShort(image));
                      tile_image->colormap[j].blue=(Quantum)
                        ScaleShortToQuantum(ReadBlobMSBShort(image));
                    }
                  }
                else
                  {
                    for (i=0; i < (ssize_t) tile_image->colors; i++)
                    {
                      tile_image->colormap[i].red=(Quantum) (QuantumRange-
                        tile_image->colormap[i].red);
                      tile_image->colormap[i].green=(Quantum) (QuantumRange-
                        tile_image->colormap[i].green);
                      tile_image->colormap[i].blue=(Quantum) (QuantumRange-
                        tile_image->colormap[i].blue);
                    }
                  }
              }
            if (ReadRectangle(image,&source) == MagickFalse)
              ThrowPICTException(CorruptImageError,"ImproperImageHeader");
            if (ReadRectangle(image,&destination) == MagickFalse)
              ThrowPICTException(CorruptImageError,"ImproperImageHeader");
            (void) ReadBlobMSBShort(image);
            if ((code == 0x91) || (code == 0x99) || (code == 0x9b))
              {
                /*
                  Skip region.
                */
                length=ReadBlobMSBShort(image);
                for (i=0; i < (ssize_t) (length-2); i++)
                  if (ReadBlobByte(image) == EOF)
                    break;
              }
            if ((code != 0x9a) && (code != 0x9b) &&
                (bytesperrow & 0x8000) == 0)
              pixels=DecodeImage(image,tile_image,(size_t) bytesperrow,1,
                &extent,exception);
            else
              pixels=DecodeImage(image,tile_image,(size_t) bytesperrow,
                (unsigned int) pixmap.bits_per_pixel,&extent,exception);
            if (pixels == (unsigned char *) NULL)
              ThrowPICTException(ResourceLimitError,"MemoryAllocationFailed");
            /*
              Convert PICT tile image to pixel packets.
            */
            p=pixels;
            for (y=0; y < (ssize_t) tile_image->rows; y++)
            {
              if (p > (pixels+extent+image->columns))
                ThrowPICTException(CorruptImageError,"NotEnoughPixelData");
              q=QueueAuthenticPixels(tile_image,0,y,tile_image->columns,1,
                exception);
              if (q == (Quantum *) NULL)
                break;
              for (x=0; x < (ssize_t) tile_image->columns; x++)
              {
                if (tile_image->storage_class == PseudoClass)
                  {
                    index=(Quantum) ConstrainColormapIndex(tile_image,(ssize_t)
                      *p,exception);
                    SetPixelIndex(tile_image,index,q);
                    SetPixelRed(tile_image,
                      tile_image->colormap[(ssize_t) index].red,q);
                    SetPixelGreen(tile_image,
                      tile_image->colormap[(ssize_t) index].green,q);
                    SetPixelBlue(tile_image,
                      tile_image->colormap[(ssize_t) index].blue,q);
                  }
                else
                  {
                    if (pixmap.bits_per_pixel == 16)
                      {
                        i=(ssize_t) (*p++);
                        j=(size_t) (*p);
                        SetPixelRed(tile_image,ScaleCharToQuantum(
                          (unsigned char) ((i & 0x7c) << 1)),q);
                        SetPixelGreen(tile_image,ScaleCharToQuantum(
                          (unsigned char) (((i & 0x03) << 6) |
                          ((j & 0xe0) >> 2))),q);
                        SetPixelBlue(tile_image,ScaleCharToQuantum(
                          (unsigned char) ((j & 0x1f) << 3)),q);
                      }
                    else
                      if (tile_image->alpha_trait == UndefinedPixelTrait)
                        {
                          if (p > (pixels+extent+2*image->columns))
                            ThrowPICTException(CorruptImageError,
                              "NotEnoughPixelData");
                          SetPixelRed(tile_image,ScaleCharToQuantum(*p),q);
                          SetPixelGreen(tile_image,ScaleCharToQuantum(
                            *(p+tile_image->columns)),q);
                          SetPixelBlue(tile_image,ScaleCharToQuantum(
                            *(p+2*tile_image->columns)),q);
                        }
                      else
                        {
                          if (p > (pixels+extent+3*image->columns))
                            ThrowPICTException(CorruptImageError,
                              "NotEnoughPixelData");
                          SetPixelAlpha(tile_image,ScaleCharToQuantum(*p),q);
                          SetPixelRed(tile_image,ScaleCharToQuantum(
                            *(p+tile_image->columns)),q);
                          SetPixelGreen(tile_image,ScaleCharToQuantum(
                            *(p+2*tile_image->columns)),q);
                          SetPixelBlue(tile_image,ScaleCharToQuantum(
                            *(p+3*tile_image->columns)),q);
                        }
                  }
                p++;
                q+=GetPixelChannels(tile_image);
              }
              if (SyncAuthenticPixels(tile_image,exception) == MagickFalse)
                break;
              if ((tile_image->storage_class == DirectClass) &&
                  (pixmap.bits_per_pixel != 16))
                {
                  p+=(pixmap.component_count-1)*tile_image->columns;
                  if (p < pixels)
                    break;
                }
              status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
                tile_image->rows);
              if (status == MagickFalse)
                break;
            }
            pixels=(unsigned char *) RelinquishMagickMemory(pixels);
            if ((jpeg == MagickFalse) && (EOFBlob(image) == MagickFalse))
              if ((code == 0x9a) || (code == 0x9b) ||
                  ((bytesperrow & 0x8000) != 0))
                (void) CompositeImage(image,tile_image,CopyCompositeOp,
                  MagickTrue,(ssize_t) destination.left,(ssize_t)
                  destination.top,exception);
            tile_image=DestroyImage(tile_image);
            break;