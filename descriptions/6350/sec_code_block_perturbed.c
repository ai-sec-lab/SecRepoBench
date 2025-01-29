if (status == MagickFalse)
      ThrowPCXException(exception->severity,exception->reason);
    (void) SetImageBackgroundColor(image,exception);
    (void) memset(pcx_colormap,0,sizeof(pcx_colormap));
    count=ReadBlob(image,3*image->colors,pcx_colormap);
    if (count != (ssize_t) (3*image->colors))
      ThrowPCXException(CorruptImageError,"ImproperImageHeader");