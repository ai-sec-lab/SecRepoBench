ChannelMaskTlv channelMaskTlv;

    SuccessOrExit(GetTlv(aMessage, kChannelMask, sizeof(channelMaskTlv), channelMaskTlv));
    mask = channelMaskTlv.GetChannelMask();