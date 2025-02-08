png[15]=(unsigned char) (icon_info.bits_per_pixel >> 8);
        count=ReadBlob(image,length-16,png+16);
        icon_image=(Image *) NULL;
        if (count > 0)
          {
            read_info=CloneImageInfo(image_info);
            (void) CopyMagickString(read_info->magick,"PNG",MagickPathExtent);
            icon_image=BlobToImage(read_info,png,length+16,exception);
            read_info=DestroyImageInfo(read_info);
          }