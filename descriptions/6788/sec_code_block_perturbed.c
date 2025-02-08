(void) memset(&info,0,sizeof(info));
  data=(unsigned char *) NULL;
  graymap=(int *) NULL;
  redmap=(int *) NULL;
  greenmap=(int *) NULL;
  bluemap=(int *) NULL;
  stream_info=(DCMStreamInfo *) AcquireMagickMemory(sizeof(*stream_info));
  if (stream_info == (DCMStreamInfo *) NULL)
    ThrowDCMException(ResourceLimitError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(stream_info,0,sizeof(*stream_info));
  count=ReadBlob(image,128,(unsigned char *) magick);
  if (count != 128)
    ThrowDCMException(CorruptImageError,"ImproperImageHeader");
  count=ReadBlob(image,4,(unsigned char *) magick);
  if ((count != 4) || (LocaleNCompare(magick,"DICM",4) != 0))
    {
      offset=SeekBlob(image,0L,SEEK_SET);
      if (offset < 0)
        ThrowDCMException(CorruptImageError,"ImproperImageHeader");
    }
  /*
    Read DCM Medical image.
  */
  (void) CopyMagickString(photometric,"MONOCHROME1 ",MagickPathExtent);
  info.bits_allocated=8;