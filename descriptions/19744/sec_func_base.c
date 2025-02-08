static MagickBooleanType CheckPSDChannels(const PSDInfo *psd_info,
  LayerInfo *layer_info)
{
  int
    channel_type;

  register ssize_t
    i;

  if (layer_info->channels < psd_info->min_channels)
    return(MagickFalse);
  channel_type=RedChannel;
  if (psd_info->min_channels >= 3)
    channel_type|=(GreenChannel | BlueChannel);
  if (psd_info->min_channels >= 4)
    channel_type|=BlackChannel;
  for (i=0; i < (ssize_t) layer_info->channels; i++)
  {
    short
      type;

    type=layer_info->channel_info[i].type;
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
  }
  if (channel_type == 0)
    return(MagickTrue);
  if ((channel_type == AlphaChannel) &&
      (layer_info->channels >= psd_info->min_channels + 1))
    return(MagickTrue);
  return(MagickFalse);
}