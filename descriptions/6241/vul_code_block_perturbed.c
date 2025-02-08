pixels=(unsigned char *) GetVirtualMemoryBlob(pixel_info);
    (void) ResetMagickMemory(pixels,0,(size_t) pcx_packets*(2*sizeof(*pixels)));
    /*
      Uncompress image data.
    */
    p=pixels;
    if (pcx_info.encoding == 0)
      while (pcx_packets != 0)
      {
        packet=(unsigned char) ReadBlobByte(image);
        if (EOFBlob(image) != MagickFalse)
          ThrowPCXException(CorruptImageError,"UnexpectedEndOfFile");
        *p++=packet;
        pcx_packets--;
      }
    else
      while (pcx_packets != 0)
      {
        packet=(unsigned char) ReadBlobByte(image);
        if (EOFBlob(image) != MagickFalse)
          ThrowPCXException(CorruptImageError,"UnexpectedEndOfFile");
        if ((packet & 0xc0) != 0xc0)
          {
            *p++=packet;
            pcx_packets--;
            continue;
          }
        count=(ssize_t) (packet & 0x3f);
        packet=(unsigned char) ReadBlobByte(image);
        if (EOFBlob(image) != MagickFalse)
          ThrowPCXException(CorruptImageError,"UnexpectedEndOfFile");
        for ( ; count != 0; count--)
        {
          *p++=packet;
          pcx_packets--;
          if (pcx_packets == 0)
            break;
        }
      }
    if (image->storage_class == DirectClass)
      image->alpha_trait=pcx_info.planes > 3 ? BlendPixelTrait :
        UndefinedPixelTrait;
    else
      if ((pcx_info.version == 5) ||
          ((pcx_info.bits_per_pixel*pcx_info.planes) == 1))
        {
          /*
            Initialize image colormap.
          */
          if (image->colors > 256)
            ThrowPCXException(CorruptImageError,"ColormapExceeds256Colors");
          if ((pcx_info.bits_per_pixel*pcx_info.planes) == 1)
            {
              /*
                Monochrome colormap.
              */
              image->colormap[0].red=(Quantum) 0;
              image->colormap[0].green=(Quantum) 0;
              image->colormap[0].blue=(Quantum) 0;
              image->colormap[1].red=QuantumRange;
              image->colormap[1].green=QuantumRange;
              image->colormap[1].blue=QuantumRange;
            }
          else
            if (image->colors > 16)
              {
                /*
                  256 color images have their color map at the end of the file.
                */
                pcx_info.colormap_signature=(unsigned char) ReadBlobByte(image);
                count=ReadBlob(image,3*image->colors,pcx_colormap);
                p=pcx_colormap;
                for (i=0; i < (ssize_t) image->colors; i++)
                {
                  image->colormap[i].red=ScaleCharToQuantum(*p++);
                  image->colormap[i].green=ScaleCharToQuantum(*p++);
                  image->colormap[i].blue=ScaleCharToQuantum(*p++);
                }
            }
        }
    /*
      Convert PCX raster image to pixel packets.
    */
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      p=pixels+(y*pcx_info.bytes_per_line*pcx_info.planes);
      q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
      if (q == (Quantum *) NULL)
        break;
      r=scanline;
      if (image->storage_class == DirectClass)
        for (i=0; i < pcx_info.planes; i++)
        {
          r=scanline+i;
          for (x=0; x < (ssize_t) pcx_info.bytes_per_line; x++)
          {
            switch (i)
            {
              case 0:
              {
                *r=(*p++);
                break;
              }
              case 1:
              {
                *r=(*p++);
                break;
              }
              case 2:
              {
                *r=(*p++);
                break;
              }
              case 3:
              default:
              {
                *r=(*p++);
                break;
              }
            }
            r+=pcx_info.planes;
          }
        }
      else
        if (pcx_info.planes > 1)
          {
            for (x=0; x < (ssize_t) image->columns; x++)
              *r++=0;
            for (i=0; i < pcx_info.planes; i++)
            {
              r=scanline;
              for (x=0; x < (ssize_t) pcx_info.bytes_per_line; x++)
              {
                 bitmask=(*p++);
                 for (mask=0x80; mask != 0; mask>>=1)
                 {
                   if (bitmask & mask)
                     *r|=1 << i;
                   r++;
                 }
               }
            }
          }
        else
          switch (pcx_info.bits_per_pixel)
          {
            case 1:
            {
              register ssize_t
                bit;

              for (x=0; x < ((ssize_t) image->columns-7); x+=8)
              {
                for (bit=7; bit >= 0; bit--)
                  *r++=(unsigned char) ((*p) & (0x01 << bit) ? 0x01 : 0x00);
                p++;
              }
              if ((image->columns % 8) != 0)
                {
                  for (bit=7; bit >= (ssize_t) (8-(image->columns % 8)); bit--)
                    *r++=(unsigned char) ((*p) & (0x01 << bit) ? 0x01 : 0x00);
                  p++;
                }
              break;
            }
            case 2:
            {
              for (x=0; x < ((ssize_t) image->columns-3); x+=4)
              {
                *r++=(*p >> 6) & 0x3;
                *r++=(*p >> 4) & 0x3;
                *r++=(*p >> 2) & 0x3;
                *r++=(*p) & 0x3;
                p++;
              }
              if ((image->columns % 4) != 0)
                {
                  for (i=3; i >= (ssize_t) (4-(image->columns % 4)); i--)
                    *r++=(unsigned char) ((*p >> (i*2)) & 0x03);
                  p++;
                }
              break;
            }
            case 4:
            {
              for (x=0; x < ((ssize_t) image->columns-1); x+=2)
              {
                *r++=(*p >> 4) & 0xf;
                *r++=(*p) & 0xf;
                p++;
              }
              if ((image->columns % 2) != 0)
                *r++=(*p++ >> 4) & 0xf;
              break;
            }
            case 8:
            {
              (void) CopyMagickMemory(r,p,image->columns);
              break;
            }
            default:
              break;
          }
      /*
        Transfer image scanline.
      */
      r=scanline;
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        if (image->storage_class == PseudoClass)
          SetPixelIndex(image,*r++,q);
        else
          {
            SetPixelRed(image,ScaleCharToQuantum(*r++),q);
            SetPixelGreen(image,ScaleCharToQuantum(*r++),q);
            SetPixelBlue(image,ScaleCharToQuantum(*r++),q);
            if (image->alpha_trait != UndefinedPixelTrait)
              SetPixelAlpha(image,ScaleCharToQuantum(*r++),q);
          }
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
    if (image->storage_class == PseudoClass)
      (void) SyncImage(image,exception);
    scanline=(unsigned char *) RelinquishMagickMemory(scanline);
    pixel_info=RelinquishVirtualMemory(pixel_info);
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
    if (page_table == (MagickOffsetType *) NULL)
      break;
    if (page_table[id] == 0)
      break;
    offset=SeekBlob(image,(MagickOffsetType) page_table[id],SEEK_SET);
    if (offset < 0)
      ThrowPCXException(CorruptImageError,"ImproperImageHeader");
    count=ReadBlob(image,1,&pcx_info.identifier);
    if ((count != 0) && (pcx_info.identifier == 0x0a))
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