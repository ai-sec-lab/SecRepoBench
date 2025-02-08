otError MeshForwarder::GetFramePriority(uint8_t *           aFrame,
                                        uint8_t             frameLen,
                                        const Mac::Address &aMacSource,
                                        const Mac::Address &aMacDest,
                                        uint8_t &           aPriority)
{
    otError        error = OT_ERROR_NONE;
    Ip6::Header    ip6Header;
    Ip6::UdpHeader udpHeader;
    uint8_t        headerLength;
    bool           nextHeaderCompressed;

    SuccessOrExit(error = DecompressIp6Header(aFrame, frameLen, aMacSource, aMacDest, ip6Header, headerLength,
                                              nextHeaderCompressed));
    aPriority = GetNetif().GetIp6().DscpToPriority(ip6Header.GetDscp());
    VerifyOrExit(ip6Header.GetNextHeader() == Ip6::kProtoUdp);

    aFrame += headerLength;
    frameLen -= headerLength;

    if (nextHeaderCompressed)
    {
        VerifyOrExit(GetNetif().GetLowpan().DecompressUdpHeader(udpHeader, aFrame, frameLen) >= 0);
    }
    else
    {
        VerifyOrExit(frameLen >= sizeof(Ip6::UdpHeader), error = OT_ERROR_PARSE);
        memcpy(&udpHeader, aFrame, sizeof(Ip6::UdpHeader));
    }

    if (udpHeader.GetDestinationPort() == Mle::kUdpPort || udpHeader.GetDestinationPort() == kCoapUdpPort)
    {
        aPriority = Message::kPriorityNet;
    }

exit:
    return error;
}