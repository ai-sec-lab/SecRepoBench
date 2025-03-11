for ( ; offset < (MagickOffsetType) dpx.file.image_offset; offset++)
    if (ReadBlobByte(image) == EOF)
      break;
  if (EOFBlob(image) != MagickFalse)
    ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  if (image_info->ping != MagickFalse)
    {
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  operationsuccess=SetImageExtent(image,image->columns,image->rows,exception);