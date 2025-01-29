if (status == MagickFalse)
      return(DestroyImageList(image));
    /*
      Convert PIX raster image to pixel packets.
    */
    red=(Quantum) 0;
    green=(Quantum) 0;
    blue=(Quantum) 0;
    index=0;
    length=0;
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
      if (q == (Quantum *) NULL)
        break;
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        if (length == 0)
          {
            int
              c;

            c=ReadBlobByte(image);
            if ((c == 0) || (c == EOF))
              break;
            length=(size_t) c;
            if (bits_per_pixel == 8)
              index=ScaleCharToQuantum((unsigned char) ReadBlobByte(image));
            else
              {
                blue=ScaleCharToQuantum((unsigned char) ReadBlobByte(image));
                green=ScaleCharToQuantum((unsigned char) ReadBlobByte(image));
                red=ScaleCharToQuantum((unsigned char) ReadBlobByte(image));
              }
          }
        if (image->storage_class == PseudoClass)
          SetPixelIndex(image,index,q);
        SetPixelBlue(image,blue,q);
        SetPixelGreen(image,green,q);
        SetPixelRed(image,red,q);
        length--;
        q+=GetPixelChannels(image);
      }
      if (x < (ssize_t) image->columns)
        break;
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
    width=ReadBlobMSBLong(image);
    height=ReadBlobMSBLong(image);
    (void) ReadBlobMSBShort(image);
    (void) ReadBlobMSBShort(image);
    bits_per_pixel=ReadBlobMSBShort(image);
    status=(width != 0UL) && (height == 0UL) && ((bits_per_pixel == 8) ||
      (bits_per_pixel == 24)) ? MagickTrue : MagickFalse;
    if (status != MagickFalse)
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