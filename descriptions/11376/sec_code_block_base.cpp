if (nextHeaderCompressed)
    {
        VerifyOrExit(GetNetif().GetLowpan().DecompressUdpHeader(udpHeader, aFrame, aFrameLength) >= 0);
    }
    else
    {
        VerifyOrExit(aFrameLength >= sizeof(Ip6::UdpHeader), error = OT_ERROR_PARSE);
        memcpy(&udpHeader, aFrame, sizeof(Ip6::UdpHeader));
    }

    if (udpHeader.GetDestinationPort() == Mle::kUdpPort || udpHeader.GetDestinationPort() == kCoapUdpPort)
    {
        aPriority = Message::kPriorityNet;
    }

exit:
    return error;