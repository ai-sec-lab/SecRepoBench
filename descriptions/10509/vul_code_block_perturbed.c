image->depth=GetImageDepth(image,exception);
  for (j=0; j<6; j++)
  {
    /*
     * Sometimes we get DirectClass images that have 256 colors or fewer.
     * This code will build a colormap.
     *
     * Also, sometimes we get PseudoClass images with an out-of-date
     * colormap.  This code will replace the colormap with a new one.
     * Sometimes we get PseudoClass images that have more than 256 colors.
     * This code will delete the colormap and change the image to
     * DirectClass.
     *
     * If image->alpha_trait is MagickFalse, we ignore the alpha channel
     * even though it sometimes contains left-over non-opaque values.
     *
     * Also we gather some information (number of opaque, transparent,
     * and semitransparent pixels, and whether the image has any non-gray
     * pixels or only black-and-white pixels) that we might need later.
     *
     * Even if the user wants to force GrayAlpha or RGBA (colortype 4 or 6)
     * we need to check for bogus non-opaque values, at least.
     */

   int
     n;

   PixelInfo
     opaque[260],
     semitransparent[260],
     transparent[260];

   register const Quantum
     *r;

   register Quantum
     *q;

   if (logging != MagickFalse)
     (void) LogMagickEvent(CoderEvent,GetMagickModule(),
         "    Enter BUILD_PALETTE:");

   if (logging != MagickFalse)
     {
       (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "      image->columns=%.20g",(double) image->columns);
       (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "      image->rows=%.20g",(double) image->rows);
       (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "      image->alpha_trait=%.20g",(double) image->alpha_trait);
       (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "      image->depth=%.20g",(double) image->depth);

       if (image->storage_class == PseudoClass && image->colormap != NULL)
       {
         (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "      Original colormap:");
         (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "        i    (red,green,blue,alpha)");

         for (i=0; i < 256; i++)
         {
               (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "        %d    (%d,%d,%d,%d)",
                    (int) i,
                    (int) image->colormap[i].red,
                    (int) image->colormap[i].green,
                    (int) image->colormap[i].blue,
                    (int) image->colormap[i].alpha);
         }

         for (i=image->colors - 10; i < (ssize_t) image->colors; i++)
         {
           if (i > 255)
             {
               (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "        %d    (%d,%d,%d,%d)",
                    (int) i,
                    (int) image->colormap[i].red,
                    (int) image->colormap[i].green,
                    (int) image->colormap[i].blue,
                    (int) image->colormap[i].alpha);
             }
         }
       }

       (void) LogMagickEvent(CoderEvent,GetMagickModule(),
           "      image->colors=%d",(int) image->colors);

       if (image->colors == 0)
         (void) LogMagickEvent(CoderEvent,GetMagickModule(),
             "        (zero means unknown)");

       if (ping_preserve_colormap == MagickFalse)
         (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "      Regenerate the colormap");
     }

     image_colors=0;
     number_opaque = 0;
     number_semitransparent = 0;
     number_transparent = 0;

     for (y=0; y < (ssize_t) image->rows; y++)
     {
       r=GetVirtualPixels(image,0,y,image->columns,1,exception);

       if (r == (const Quantum *) NULL)
         break;

       for (x=0; x < (ssize_t) image->columns; x++)
       {
           if (image->alpha_trait == UndefinedPixelTrait ||
              GetPixelAlpha(image,r) == OpaqueAlpha)
             {
               if (number_opaque < 259)
                 {
                   if (number_opaque == 0)
                     {
                       GetPixelInfoPixel(image,r,opaque);
                       opaque[0].alpha=OpaqueAlpha;
                       number_opaque=1;
                     }

                   for (i=0; i< (ssize_t) number_opaque; i++)
                     {
                       if (IsColorEqual(image,r,opaque+i))
                         break;
                     }

                   if (i ==  (ssize_t) number_opaque && number_opaque < 259)
                     {
                       number_opaque++;
                       GetPixelInfoPixel(image,r,opaque+i);
                       opaque[i].alpha=OpaqueAlpha;
                     }
                 }
             }
           else if (GetPixelAlpha(image,r) == TransparentAlpha)
             {
               if (number_transparent < 259)
                 {
                   if (number_transparent == 0)
                     {
                       GetPixelInfoPixel(image,r,transparent);
                       ping_trans_color.red=(unsigned short)
                         GetPixelRed(image,r);
                       ping_trans_color.green=(unsigned short)
                         GetPixelGreen(image,r);
                       ping_trans_color.blue=(unsigned short)
                         GetPixelBlue(image,r);
                       ping_trans_color.gray=(unsigned short)
                         GetPixelGray(image,r);
                       number_transparent = 1;
                     }

                   for (i=0; i< (ssize_t) number_transparent; i++)
                     {
                       if (IsColorEqual(image,r,transparent+i))
                         break;
                     }

                   if (i ==  (ssize_t) number_transparent &&
                       number_transparent < 259)
                     {
                       number_transparent++;
                       GetPixelInfoPixel(image,r,transparent+i);
                     }
                 }
             }
           else
             {
               if (number_semitransparent < 259)
                 {
                   if (number_semitransparent == 0)
                     {
                       GetPixelInfoPixel(image,r,semitransparent);
                       number_semitransparent = 1;
                     }

                   for (i=0; i< (ssize_t) number_semitransparent; i++)
                     {
                       if (IsColorEqual(image,r,semitransparent+i)
                           && GetPixelAlpha(image,r) ==
                           semitransparent[i].alpha)
                         break;
                     }

                   if (i ==  (ssize_t) number_semitransparent &&
                       number_semitransparent < 259)
                     {
                       number_semitransparent++;
                       GetPixelInfoPixel(image,r,semitransparent+i);
                     }
                 }
             }
           r+=GetPixelChannels(image);
        }
     }

     if (mng_info->write_png8 == MagickFalse &&
         ping_exclude_bKGD == MagickFalse)
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
          for (i=0; i<number_opaque; i++)
          {
             if (opaque[i].red == image->background_color.red &&
                 opaque[i].green == image->background_color.green &&
                 opaque[i].blue == image->background_color.blue)
               break;
          }
          if (number_opaque < 259 && i == number_opaque)
            {
               opaque[i] = image->background_color;
               ping_background.index = i;
               number_opaque++;
               if (logging != MagickFalse)
                 {
                   (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                       "      background_color index is %d",(int) i);
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

     if (ping_preserve_colormap != MagickFalse)
       break;

     if (mng_info->write_png_colortype != 7) /* We won't need this info */
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

         if(image_colors > 256)
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

     if (image_colors < 257)
       {
         PixelInfo
           colormap[260];

         /*
          * Initialize image colormap.
          */

         if (logging != MagickFalse)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                  "      Sort the new colormap");

        /* Sort palette, transparent first */;

         n = 0;

         for (i=0; i<number_transparent; i++)
            colormap[n++] = transparent[i];

         for (i=0; i<number_semitransparent; i++)
            colormap[n++] = semitransparent[i];

         for (i=0; i<number_opaque; i++)
            colormap[n++] = opaque[i];

         ping_background.index +=
           (number_transparent + number_semitransparent);

         /* image_colors < 257; search the colormap instead of the pixels
          * to get ping_have_color and ping_have_non_bw
          */
         for (i=0; i<n; i++)
         {
           if (ping_have_color == MagickFalse)
             {
                if (colormap[i].red != colormap[i].green ||
                    colormap[i].red != colormap[i].blue)
                  {
                     ping_have_color=MagickTrue;
                     ping_have_non_bw=MagickTrue;
                     break;
                  }
              }

           if (ping_have_non_bw == MagickFalse)
             {
               if (colormap[i].red != 0 && colormap[i].red != QuantumRange)
                   ping_have_non_bw=MagickTrue;
             }
          }

        if ((mng_info->ping_exclude_tRNS == MagickFalse ||
            (number_transparent == 0 && number_semitransparent == 0)) &&
            (((mng_info->write_png_colortype-1) ==
            PNG_COLOR_TYPE_PALETTE) ||
            (mng_info->write_png_colortype == 0)))
          {
            if (logging != MagickFalse)
              {
                if (n !=  (ssize_t) image_colors)
                   (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "   image_colors (%d) and n (%d)  don't match",
                   image_colors, n);

                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "      AcquireImageColormap");
              }

            image->colors = image_colors;

            if (AcquireImageColormap(image,image_colors,exception) == MagickFalse)
              {
                (void) ThrowMagickException(exception,GetMagickModule(),
                  ResourceLimitError,"MemoryAllocationFailed","`%s'",
                  image->filename);
                break;
              }

            for (i=0; i< (ssize_t) image_colors; i++)
               image->colormap[i] = colormap[i];

            if (logging != MagickFalse)
              {
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                      "      image->colors=%d (%d)",
                      (int) image->colors, image_colors);

                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                      "      Update the pixel indexes");
              }

            /* Sync the pixel indices with the new colormap */

            for (y=0; y < (ssize_t) image->rows; y++)
            {
              q=GetAuthenticPixels(image,0,y,image->columns,1,exception);

              if (q == (Quantum *) NULL)
                break;

              for (x=0; x < (ssize_t) image->columns; x++)
              {
                for (i=0; i< (ssize_t) image_colors; i++)
                {
                  if ((image->alpha_trait == UndefinedPixelTrait ||
                      image->colormap[i].alpha == GetPixelAlpha(image,q)) &&
                      image->colormap[i].red == GetPixelRed(image,q) &&
                      image->colormap[i].green == GetPixelGreen(image,q) &&
                      image->colormap[i].blue == GetPixelBlue(image,q))
                  {
                    SetPixelIndex(image,i,q);
                    break;
                  }
                }
                q+=GetPixelChannels(image);
              }

              if (SyncAuthenticPixels(image,exception) == MagickFalse)
                 break;
            }
          }
       }

     if (logging != MagickFalse)
       {
         (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "      image->colors=%d", (int) image->colors);

         if (image->colormap != NULL)
           {
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                 "       i     (red,green,blue,alpha)");

             for (i=0; i < (ssize_t) image->colors; i++)
             {
               if (i < 300 || i >= (ssize_t) image->colors - 10)
                 {
                   (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                       "       %d     (%d,%d,%d,%d)",
                        (int) i,
                        (int) image->colormap[i].red,
                        (int) image->colormap[i].green,
                        (int) image->colormap[i].blue,
                        (int) image->colormap[i].alpha);
                 }
             }
           }

           if (number_transparent < 257)
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "      number_transparent     = %d",
                   number_transparent);
           else

             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "      number_transparent     > 256");

           if (number_opaque < 257)
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "      number_opaque          = %d",
                   number_opaque);

           else
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "      number_opaque          > 256");

           if (number_semitransparent < 257)
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "      number_semitransparent = %d",
                   number_semitransparent);

           else
             (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                   "      number_semitransparent > 256");

           if (ping_have_non_bw == MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "      All pixels and the background are black or white");

           else if (ping_have_color == MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "      All pixels and the background are gray");

           else
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                    "      At least one pixel or the background is non-gray");

           (void) LogMagickEvent(CoderEvent,GetMagickModule(),
               "    Exit BUILD_PALETTE:");
       }

   if (mng_info->write_png8 == MagickFalse)
      break;

   /* Make any reductions necessary for the PNG8 format */
    if (image_colors <= 256 &&
        image_colors != 0 && image->colormap != NULL &&
        number_semitransparent == 0 &&
        number_transparent <= 1)
      break;

    /* PNG8 can't have semitransparent colors so we threshold the
     * opacity to 0 or OpaqueOpacity, and PNG8 can only have one
     * transparent color so if more than one is transparent we merge
     * them into image->background_color.
     */
    if (number_semitransparent != 0 || number_transparent > 1)
      {
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Thresholding the alpha channel to binary");

        for (y=0; y < (ssize_t) image->rows; y++)
        {
          q=GetAuthenticPixels(image,0,y,image->columns,1,exception);

          if (q == (Quantum *) NULL)
            break;

          for (x=0; x < (ssize_t) image->columns; x++)
          {
              if (GetPixelAlpha(image,q) < OpaqueAlpha/2)
                {
                  SetPixelViaPixelInfo(image,&image->background_color,q);
                  SetPixelAlpha(image,TransparentAlpha,q);
                }
              else
                  SetPixelAlpha(image,OpaqueAlpha,q);
              q+=GetPixelChannels(image);
          }

          if (SyncAuthenticPixels(image,exception) == MagickFalse)
             break;

          if (image_colors != 0 && image_colors <= 256 &&
             image->colormap != NULL)
            for (i=0; i<image_colors; i++)
                image->colormap[i].alpha =
                    (image->colormap[i].alpha > TransparentAlpha/2 ?
                    TransparentAlpha : OpaqueAlpha);
        }
      continue;
    }

    /* PNG8 can't have more than 256 colors so we quantize the pixels and
     * background color to the 4-4-4-1, 3-3-3-1 or 3-3-2-1 palette.  If the
     * image is mostly gray, the 4-4-4-1 palette is likely to end up with 256
     * colors or less.
     */
    if (tried_444 == MagickFalse && (image_colors == 0 || image_colors > 256))
      {
        if (logging != MagickFalse)
           (void) LogMagickEvent(CoderEvent,GetMagickModule(),
               "    Quantizing the background color to 4-4-4");

        tried_444 = MagickTrue;

        LBR04PacketRGB(image->background_color);

        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    Quantizing the pixel colors to 4-4-4");

        if (image->colormap == NULL)
        {
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            q=GetAuthenticPixels(image,0,y,image->columns,1,exception);

            if (q == (Quantum *) NULL)
              break;

            for (x=0; x < (ssize_t) image->columns; x++)
            {
              if (GetPixelAlpha(image,q) == OpaqueAlpha)
                  LBR04PixelRGB(q);
              q+=GetPixelChannels(image);
            }

            if (SyncAuthenticPixels(image,exception) == MagickFalse)
               break;
          }
        }

        else /* Should not reach this; colormap already exists and
                must be <= 256 */
        {
          if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    Quantizing the colormap to 4-4-4");

          for (i=0; i<image_colors; i++)
          {
            LBR04PacketRGB(image->colormap[i]);
          }
        }
        continue;
      }

    if (tried_333 == MagickFalse && (image_colors == 0 || image_colors > 256))
      {
        if (logging != MagickFalse)
           (void) LogMagickEvent(CoderEvent,GetMagickModule(),
               "    Quantizing the background color to 3-3-3");

        tried_333 = MagickTrue;

        LBR03PacketRGB(image->background_color);

        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    Quantizing the pixel colors to 3-3-3-1");

        if (image->colormap == NULL)
        {
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            q=GetAuthenticPixels(image,0,y,image->columns,1,exception);

            if (q == (Quantum *) NULL)
              break;

            for (x=0; x < (ssize_t) image->columns; x++)
            {
              if (GetPixelAlpha(image,q) == OpaqueAlpha)
                  LBR03RGB(q);
              q+=GetPixelChannels(image);
            }

            if (SyncAuthenticPixels(image,exception) == MagickFalse)
               break;
          }
        }

        else /* Should not reach this; colormap already exists and
                must be <= 256 */
        {
          if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    Quantizing the colormap to 3-3-3-1");
          for (i=0; i<image_colors; i++)
          {
              LBR03PacketRGB(image->colormap[i]);
          }
        }
        continue;
      }

    if (tried_332 == MagickFalse && (image_colors == 0 || image_colors > 256))
      {
        if (logging != MagickFalse)
           (void) LogMagickEvent(CoderEvent,GetMagickModule(),
               "    Quantizing the background color to 3-3-2");

        tried_332 = MagickTrue;

        /* Red and green were already done so we only quantize the blue
         * channel
         */

        LBR02PacketBlue(image->background_color);

        if (logging != MagickFalse)
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    Quantizing the pixel colors to 3-3-2-1");

        if (image->colormap == NULL)
        {
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            q=GetAuthenticPixels(image,0,y,image->columns,1,exception);

            if (q == (Quantum *) NULL)
              break;

            for (x=0; x < (ssize_t) image->columns; x++)
            {
              if (GetPixelAlpha(image,q) == OpaqueAlpha)
                  LBR02PixelBlue(q);
              q+=GetPixelChannels(image);
            }

            if (SyncAuthenticPixels(image,exception) == MagickFalse)
               break;
          }
        }

        else /* Should not reach this; colormap already exists and
                must be <= 256 */
        {
          if (logging != MagickFalse)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
              "    Quantizing the colormap to 3-3-2-1");
          for (i=0; i<image_colors; i++)
          {
              LBR02PacketBlue(image->colormap[i]);
          }
      }
      continue;
    }

    if (image_colors == 0 || image_colors > 256)
    {
      /* Take care of special case with 256 opaque colors + 1 transparent
       * color.  We don't need to quantize to 2-3-2-1; we only need to
       * eliminate one color, so we'll merge the two darkest red
       * colors (0x49, 0, 0) -> (0x24, 0, 0).
       */
      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Merging two dark red background colors to 3-3-2-1");

      if (ScaleQuantumToChar(image->background_color.red) == 0x49 &&
          ScaleQuantumToChar(image->background_color.green) == 0x00 &&
          ScaleQuantumToChar(image->background_color.blue) == 0x00)
      {
         image->background_color.red=ScaleCharToQuantum(0x24);
      }

      if (logging != MagickFalse)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
            "    Merging two dark red pixel colors to 3-3-2-1");

      if (image->colormap == NULL)
      {
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          q=GetAuthenticPixels(image,0,y,image->columns,1,exception);

          if (q == (Quantum *) NULL)
            break;

          for (x=0; x < (ssize_t) image->columns; x++)
          {
            if (ScaleQuantumToChar(GetPixelRed(image,q)) == 0x49 &&
                ScaleQuantumToChar(GetPixelGreen(image,q)) == 0x00 &&
                ScaleQuantumToChar(GetPixelBlue(image,q)) == 0x00 &&
                GetPixelAlpha(image,q) == OpaqueAlpha)
              {
                SetPixelRed(image,ScaleCharToQuantum(0x24),q);
              }
            q+=GetPixelChannels(image);
          }

          if (SyncAuthenticPixels(image,exception) == MagickFalse)
             break;

        }
      }

      else
      {
         for (i=0; i<image_colors; i++)
         {
            if (ScaleQuantumToChar(image->colormap[i].red) == 0x49 &&
                ScaleQuantumToChar(image->colormap[i].green) == 0x00 &&
                ScaleQuantumToChar(image->colormap[i].blue) == 0x00)
            {
               image->colormap[i].red=ScaleCharToQuantum(0x24);
            }
         }
      }
    }
  }