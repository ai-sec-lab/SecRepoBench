if (imginfo->ping != MagickFalse)
        {
          stream=(unsigned char*) RelinquishMagickMemory(stream);
          (void) CloseBlob(image);
          return(GetFirstImageInList(image));
        }
      status=SetImageExtent(image,image->columns,image->rows,exception);
      if (status == MagickFalse)
        {
          stream=(unsigned char*) RelinquishMagickMemory(stream);
          (void) CloseBlob(image);
          return(DestroyImageList(image));
        }
      if (IsWEBPImageLossless(stream,length) != MagickFalse)
        image->quality=100;