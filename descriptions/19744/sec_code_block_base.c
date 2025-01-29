if ((i == 0) && (psd_info->mode == IndexedMode) && (type != 0))
      return(MagickFalse);
    if (type == -1)
      {
        channel_type|=AlphaChannel;
        continue;
      }
    if (type < -1)
      continue;
    if (type == 0)
      channel_type&=~RedChannel;
    else if (type == 1)
      channel_type&=~GreenChannel;
    else if (type == 2)
      channel_type&=~BlueChannel;
    else if (type == 3)
      channel_type&=~BlackChannel;