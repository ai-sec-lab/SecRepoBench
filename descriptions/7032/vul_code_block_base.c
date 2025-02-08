for (y=0; y < (ssize_t) tile_image->rows; y++)
            {
              if (p > (pixels+extent+image->columns))
                {
                  tile_image=DestroyImage(tile_image);
                  ThrowReaderException(CorruptImageError,"NotEnoughPixelData");
                }
              q=QueueAuthenticPixels(tile_image,0,y,tile_image->columns,1,
                exception);
              if (q == (Quantum *) NULL)
                break;
              for (x=0; x < (ssize_t) tile_image->columns; x++)
              {
                if (tile_image->storage_class == PseudoClass)
                  {
                    index=ConstrainColormapIndex(tile_image,*p,exception);
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
                        i=(*p++);
                        j=(*p);
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
                            {
                              tile_image=DestroyImage(tile_image);
                              ThrowReaderException(CorruptImageError,
                                "NotEnoughPixelData");
                            }
                          SetPixelRed(tile_image,ScaleCharToQuantum(*p),q);
                          SetPixelGreen(tile_image,ScaleCharToQuantum(
                            *(p+tile_image->columns)),q);
                          SetPixelBlue(tile_image,ScaleCharToQuantum(
                            *(p+2*tile_image->columns)),q);
                        }
                      else
                        {
                          if (p > (pixels+extent+3*image->columns))
                            {
                              tile_image=DestroyImage(tile_image);
                              ThrowReaderException(CorruptImageError,
                                "NotEnoughPixelData");
                            }
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
              status=SetImageProgress(image,LoadImageTag,y,tile_image->rows);
              if (status == MagickFalse)
                break;
            }
            pixels=(unsigned char *) RelinquishMagickMemory(pixels);
            if (jpeg == MagickFalse)
              if ((code == 0x9a) || (code == 0x9b) ||
                  ((bytes_per_line & 0x8000) != 0))
                (void) CompositeImage(image,tile_image,CopyCompositeOp,
                  MagickTrue,destination.left,destination.top,exception);