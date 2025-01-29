if ((photometric != PHOTOMETRIC_RGB) &&
        (photometric != PHOTOMETRIC_CIELAB) &&
        (photometric != PHOTOMETRIC_SEPARATED))
      method=ReadGenericMethod;
    if ((photometric != PHOTOMETRIC_SEPARATED) &&
        (interlace == PLANARCONFIG_SEPARATE) && (bits_per_sample < 64))
      method=ReadGenericMethod;
    if (TIFFIsTiled(tiff) != MagickFalse)
      method=ReadTileMethod;
    else
      if (TIFFGetField(tiff,TIFFTAG_ROWSPERSTRIP,&rows_per_strip) == 1)
        method=ReadStripMethod;
    if (image->compression == JPEGCompression)
      method=GetJPEGMethod(image,tiff,photometric,bits_per_sample,
        samples_per_pixel);