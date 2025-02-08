image_colors=number_opaque+number_transparent+number_semitransparent;

     if (mng_info->write_png8 == MagickFalse &&
         mng_info->exclude_bKGD == MagickFalse)
       {
         /* Add the background color to the palette, if it
          * isn't already there.
          */
          if (logging != MagickFalse)
            {
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "      Check colormap for background (%d,%d,%d)",
                  (int) image->background_color.red,
                  (int) image->background_color.green,
                  (int) image->background_color.blue);
            }
          if (number_opaque < 259)
            {
              for (i=0; i<number_opaque; i++)
              {
                 if (opaque[i].red == image->background_color.red &&
                     opaque[i].green == image->background_color.green &&
                     opaque[i].blue == image->background_color.blue)
                   break;
              }
              if ((i == number_opaque) && (image_colors < 256))
                {
                   ping_background.index=(png_byte) image->colors;
                   image->colormap[image->colors++]=image->background_color;
                   opaque[i]=image->background_color;
                   number_opaque++;
                   if (logging != MagickFalse)
                     {
                       (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                           "      background_color index is %d",(int) i);
                     }
                }
            }
          else if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "      No room in the colormap to add background color");
       }

     image_colors=number_opaque+number_transparent+number_semitransparent;

     if (logging != MagickFalse)
       {
         if (image_colors > 256)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "      image has more than 256 colors");

         else
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "      image has %d colors",image_colors);
       }

     if (mng_info->preserve_colormap != MagickFalse)
       break;

     if (mng_info->colortype != 7) /* We won't need this info */
       {
         ping_have_color=MagickFalse;
         ping_have_non_bw=MagickFalse;

         if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
         {
           (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "incompatible colorspace");
           ping_have_color=MagickTrue;
           ping_have_non_bw=MagickTrue;
         }

         if (image_colors > 256)
           {
             for (y=0; y < (ssize_t) image->rows; y++)
             {
               q=GetAuthenticPixels(image,0,y,image->columns,1,exception);

               if (q == (Quantum *) NULL)
                 break;

               r=q;
               for (x=0; x < (ssize_t) image->columns; x++)
               {
                 if (GetPixelRed(image,r) != GetPixelGreen(image,r) ||
                     GetPixelRed(image,r) != GetPixelBlue(image,r))
                   {
                      ping_have_color=MagickTrue;
                      ping_have_non_bw=MagickTrue;
                      break;
                   }
                 r+=GetPixelChannels(image);
               }

               if (ping_have_color != MagickFalse)
                 break;

               /* Worst case is black-and-white; we are looking at every
                * pixel twice.
                */

               if (ping_have_non_bw == MagickFalse)
                 {
                   r=q;
                   for (x=0; x < (ssize_t) image->columns; x++)
                   {
                     if (GetPixelRed(image,r) != 0 &&
                         GetPixelRed(image,r) != QuantumRange)
                       {
                         ping_have_non_bw=MagickTrue;
                         break;
                       }
                     r+=GetPixelChannels(image);
                   }
               }
             }
           }
       }