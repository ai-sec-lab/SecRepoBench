png=(unsigned char *) AcquireQuantumMemory(length,sizeof(*png));
        if (png == (unsigned char *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        (void) memcpy(png,"\211PNG\r\n\032\n\000\000\000\015",12);
        png[12]=(unsigned char) icon_info.planes;
        png[13]=(unsigned char) (icon_info.planes >> 8);
        png[14]=(unsigned char) icon_info.bits_per_pixel;
        png[15]=(unsigned char) (icon_info.bits_per_pixel >> 8);
        count=ReadBlob(image,length-16,png+16);
        if (count != (ssize_t) (length-16))
          {
            png=(unsigned char *) RelinquishMagickMemory(png);
            ThrowReaderException(CorruptImageError,
                "InsufficientImageDataInFile");
          }
        read_info=CloneImageInfo(image_info);
        (void) CopyMagickString(read_info->magick,"PNG",MagickPathExtent);
        icon_image=BlobToImage(read_info,png,length,exception);