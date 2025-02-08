void MleRouter::SendChildUpdateResponse(Child *                 aChild,
                                        const Ip6::MessageInfo &aMessageInfo,
                                        const uint8_t *         aTlvs,
                                        uint8_t                 aTlvsLength,
                                        const ChallengeTlv &    aChallenge)
{
    otError  error = OT_ERROR_NONE;
    Message *message;

    VerifyOrExit((message = NewMleMessage()) != NULL, error = OT_ERROR_NO_BUFS);
    SuccessOrExit(error = AppendHeader(*message, Header::kCommandChildUpdateResponse));

    for (int i = 0; i < aTlvsLength; i++)
    {
        switch (aTlvs[i])
        {
        case Tlv::kStatus:
            SuccessOrExit(error = AppendStatus(*message, StatusTlv::kError));
            break;

        case Tlv::kAddressRegistration:
            SuccessOrExit(error = AppendChildAddresses(*message, *aChild));
            break;

        case Tlv::kLeaderData:
            SuccessOrExit(error = AppendLeaderData(*message));
            break;

        case Tlv::kMode:
            SuccessOrExit(error = AppendMode(*message, aChild->GetDeviceMode()));
            break;

        // <MASK>

        case Tlv::kTimeout:
            SuccessOrExit(error = AppendTimeout(*message, aChild->GetTimeout()));
            break;

        case Tlv::kMleFrameCounter:
            SuccessOrExit(error = AppendMleFrameCounter(*message));
            break;

        case Tlv::kLinkFrameCounter:
            SuccessOrExit(error = AppendLinkFrameCounter(*message));
            break;
        }
    }

    SuccessOrExit(error = SendMessage(*message, aMessageInfo.GetPeerAddr()));

    if (aChild == NULL)
    {
        LogMleMessage("Send Child Update Response to child", aMessageInfo.GetPeerAddr());
    }
    else
    {
        LogMleMessage("Send Child Update Response to child", aMessageInfo.GetPeerAddr(), aChild->GetRloc16());
    }

exit:

    if (error != OT_ERROR_NONE && message != NULL)
    {
        message->Free();
    }
}