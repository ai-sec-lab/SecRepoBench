otError MeshForwarder::GetFramePriority(uint8_t *           aFrame,
                                        uint8_t             aFrameLength,
                                        const Mac::Address &aMacSource,
                                        const Mac::Address &aMacDest,
                                        uint8_t &           aPriority)
{
    otError        error = OT_ERROR_NONE;
    Ip6::Header    ip6Header;
    Ip6::UdpHeader udpHeader;
    uint8_t        headerLength;
    bool           nextHeaderCompressed;

    SuccessOrExit(error = DecompressIp6Header(aFrame, aFrameLength, aMacSource, aMacDest, ip6Header, headerLength,
                                              nextHeaderCompressed));
    aPriority = GetNetif().GetIp6().DscpToPriority(ip6Header.GetDscp());
    VerifyOrExit(ip6Header.GetNextHeader() == Ip6::kProtoUdp);

    aFrame += headerLength;
    aFrameLength -= headerLength;

    // <MASK>
}