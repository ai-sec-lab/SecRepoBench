ssize_t
      type;

    type=i;
    if ((type == 1) && (psd_info->channels == 2))
      type=-1;

    if (compression == RLE)
      status=ReadPSDChannelRLE(image,psd_info,type,sizes+(i*image->rows),
        exception);
    else
      status=ReadPSDChannelRaw(image,psd_info->channels,type,exception);