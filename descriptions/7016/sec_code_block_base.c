if ((image->storage_class != PseudoClass) && (bpp < 24))
                {
                  image->colors=one << bpp;
                  if (image->colors > GetBlobSize(image))
                    ThrowReaderException(CorruptImageError,
                      "InsufficientImageDataInFile");
                  if (!AcquireImageColormap(image,image->colors,exception))
                    {
                    NoMemory:
                      ThrowReaderException(ResourceLimitError,
                        "MemoryAllocationFailed");
                    }
                  /* printf("Load default colormap \n"); */
                  for (i=0; (i < (int) image->colors) && (i < 256); i++)
                    {
                      image->colormap[i].red=ScaleCharToQuantum(WPG1_Palette[i].Red);
                      image->colormap[i].green=ScaleCharToQuantum(WPG1_Palette[i].Green);
                      image->colormap[i].blue=ScaleCharToQuantum(WPG1_Palette[i].Blue);
                    }
                }
              else
                {
                  if (bpp < 24)
                    if ( (image->colors < (one << bpp)) && (bpp != 24) )
                      image->colormap=(PixelInfo *) ResizeQuantumMemory(
                        image->colormap,(size_t) (one << bpp),
                        sizeof(*image->colormap));
                }

              if ((bpp == 1) && (image->colors > 1))
                {
                  if(image->colormap[0].red==0 &&
                     image->colormap[0].green==0 &&
                     image->colormap[0].blue==0 &&
                     image->colormap[1].red==0 &&
                     image->colormap[1].green==0 &&
                     image->colormap[1].blue==0)
                    {  /* fix crippled monochrome palette */
                      image->colormap[1].red =
                        image->colormap[1].green =
                        image->colormap[1].blue = QuantumRange;
                    }
                }
              if(!image_info->ping)
                if(UnpackWPGRaster(image,bpp,exception) < 0)
                  /* The raster cannot be unpacked */
                  {
                  DecompressionFailed:
                    ThrowReaderException(CoderError,"UnableToDecompressImage");
                  }

              if(Rec.RecType==0x14 && BitmapHeader2.RotAngle!=0 && !image_info->ping)
                {
                  /* flop command */
                  if(BitmapHeader2.RotAngle & 0x8000)
                    {
                      Image
                        *flop_image;

                      flop_image = FlopImage(image, exception);
                      if (flop_image != (Image *) NULL) {
                        DuplicateBlob(flop_image,image);
                        ReplaceImageInList(&image,flop_image);
                      }
                    }
                  /* flip command */
                  if(BitmapHeader2.RotAngle & 0x2000)
                    {
                      Image
                        *flip_image;

                      flip_image = FlipImage(image, exception);
                      if (flip_image != (Image *) NULL) {
                        DuplicateBlob(flip_image,image);
                        ReplaceImageInList(&image,flip_image);
                      }
                    }
                  /* rotate command */
                  if(BitmapHeader2.RotAngle & 0x0FFF)
                    {
                      Image
                        *rotate_image;

                      rotate_image=RotateImage(image,(BitmapHeader2.RotAngle &
                        0x0FFF), exception);
                      if (rotate_image != (Image *) NULL) {
                        DuplicateBlob(rotate_image,image);
                        ReplaceImageInList(&image,rotate_image);
                      }
                    }
                }

              /* Allocate next image structure. */
              AcquireNextImage(image_info,image,exception);
              image->depth=8;
              if (image->next == (Image *) NULL)
                goto Finish;
              image=SyncNextImageInList(image);
              image->columns=image->rows=1;
              image->colors=0;
              break;