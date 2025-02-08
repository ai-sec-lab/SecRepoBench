if (compression == RLE)
      status=ReadPSDChannelRLE(image,psd_info,channelindex,sizes+(channelindex*image->rows),
        exception);
    else
      status=ReadPSDChannelRaw(image,psd_info->channels,channelindex,exception);