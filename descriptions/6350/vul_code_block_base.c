if (status == MagickFalse)
      ThrowPCXException(exception->severity,exception->reason);
    (void) SetImageBackgroundColor(image,exception);
    count=ReadBlob(image,3*image->colors,pcx_colormap);
    if (count != (ssize_t) (3*image->colors))
      ThrowPCXException(CorruptImageError,"ImproperImageHeader");