if (image_info->compression != UndefinedCompression)
        image->compression=image_info->compression;
      if (image->compression == ZipCompression)
        image->compression=RLECompression;
      if (WritePSDChannels(&psd_info,image_info,image,image,0,MagickFalse,
          exception) == 0)
        status=MagickFalse;
      image->compression=compression;