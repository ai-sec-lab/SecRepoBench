count=ReadBlob(image,length-16,png+16);
        icon_image=(Image *) NULL;
        if (count == (ssize_t) length-16)
          {
            read_info=CloneImageInfo(image_info);
            (void) CopyMagickString(read_info->magick,"PNG",MagickPathExtent);
            icon_image=BlobToImage(read_info,png,length+16,exception);
            read_info=DestroyImageInfo(read_info);
          }
        png=(unsigned char *) RelinquishMagickMemory(png);
        if (icon_image == (Image *) NULL)
          {
            if (count != (ssize_t) (length-16))
              ThrowReaderException(CorruptImageError,
                "InsufficientImageDataInFile");
            image=DestroyImageList(image);
            return((Image *) NULL);
          }
        DestroyBlob(icon_image);