status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status == MagickFalse)
      return(DestroyImageList(image));
    (void) SetImageBackgroundColor(image,exception);
    bytes_per_row=ReadBlobMSBShort(image);
    flags=ReadBlobMSBShort(image);
    bits_per_pixel=(size_t) ReadBlobByte(image);