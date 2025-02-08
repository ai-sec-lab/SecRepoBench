if (compression == RLE)
      status=ReadPSDChannelRLE(image,psd_info,i,sizes+(i*image->rows),
        exception);
    else
      status=ReadPSDChannelRaw(image,psd_info->channels,i,exception);