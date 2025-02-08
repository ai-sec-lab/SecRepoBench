count=ReadBlob(image,length,png+16);
        if (count != (ssize_t) length)
          {
            png=(unsigned char *) RelinquishMagickMemory(png);
            ThrowReaderException(CorruptImageError,
                "InsufficientImageDataInFile");
          }
        read_info=CloneImageInfo(image_info);
        (void) CopyMagickString(read_info->magick,"PNG",MagickPathExtent);
        icon_image=BlobToImage(read_info,png,length+16,exception);
        read_info=DestroyImageInfo(read_info);
        png=(unsigned char *) RelinquishMagickMemory(png);
        if (icon_image == (Image *) NULL)
          return(DestroyImageList(image));
        DestroyBlob(icon_image);