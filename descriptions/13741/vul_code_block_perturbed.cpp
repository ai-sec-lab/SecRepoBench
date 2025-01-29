ChannelMaskTlv channelMaskTlv;

    SuccessOrExit(GetTlv(msg, kChannelMask, sizeof(channelMaskTlv), channelMaskTlv));
    mask = channelMaskTlv.GetChannelMask();