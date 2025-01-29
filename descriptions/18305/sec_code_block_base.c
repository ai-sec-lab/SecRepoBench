option=GetImageOption(image_info,"tiff:exif-properties");
    if (IsStringFalse(option) == MagickFalse) /* enabled by default */
      TIFFGetEXIFProperties(tiff,image,exception);
    if ((TIFFGetFieldDefaulted(tiff,TIFFTAG_XRESOLUTION,&x_resolution) == 1) &&
        (TIFFGetFieldDefaulted(tiff,TIFFTAG_YRESOLUTION,&y_resolution) == 1))
      {
        image->resolution.x=x_resolution;
        image->resolution.y=y_resolution;
      }