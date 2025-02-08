if (nextHeaderCompressed)
    {
        VerifyOrExit(GetNetif().GetLowpan().DecompressUdpHeader(udpHeader, aFrame, frameLen) >= 0);
    }
    else
    {
        memcpy(&udpHeader, aFrame, sizeof(Ip6::UdpHeader));
    }

    if (udpHeader.GetDestinationPort() == Mle::kUdpPort || udpHeader.GetDestinationPort() == kCoapUdpPort)
    {
        aPriority = Message::kPriorityNet;
    }

exit:
    return error;