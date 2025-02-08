otError Ip6::RemoveMplOption(Message &msg)
{
    otError        error = OT_ERROR_NONE;
    Header         ip6Header;
    HopByHopHeader hbh;
    uint16_t       offset;
    uint16_t       endOffset;
    uint16_t       mplOffset = 0;
    uint8_t        mplLength = 0;
    bool           remove    = false;

    offset = 0;
    msg.Read(offset, sizeof(ip6Header), &ip6Header);
    offset += sizeof(ip6Header);
    VerifyOrExit(ip6Header.GetNextHeader() == kProtoHopOpts);

    msg.Read(offset, sizeof(hbh), &hbh);
    endOffset = offset + (hbh.GetLength() + 1) * 8;
    VerifyOrExit(msg.GetLength() >= endOffset, error = OT_ERROR_PARSE);

    offset += sizeof(hbh);

    while (offset < endOffset)
    {
        OptionHeader option;

        msg.Read(offset, sizeof(option), &option);

        switch (option.GetType())
        {
        case OptionMpl::kType:
            // if multiple MPL options exist, discard packet
            VerifyOrExit(mplOffset == 0, error = OT_ERROR_PARSE);

            // <MASK>

            offset += sizeof(option) + option.GetLength();
            break;

        case OptionPad1::kType:
            offset += sizeof(OptionPad1);
            break;

        case OptionPadN::kType:
            offset += sizeof(option) + option.GetLength();
            break;

        default:
            // encountered another option, now just replace MPL Option with PadN
            remove = false;
            offset += sizeof(option) + option.GetLength();
            break;
        }
    }

    // verify that IPv6 Options header is properly formed
    VerifyOrExit(offset == endOffset, error = OT_ERROR_PARSE);

    if (remove)
    {
        // last IPv6 Option, shrink HBH Option header
        uint8_t buf[8];

        offset = endOffset - sizeof(buf);

        while (offset >= sizeof(buf))
        {
            msg.Read(offset - sizeof(buf), sizeof(buf), buf);
            msg.Write(offset, sizeof(buf), buf);
            offset -= sizeof(buf);
        }

        msg.RemoveHeader(sizeof(buf));

        if (mplOffset == sizeof(ip6Header) + sizeof(hbh))
        {
            // remove entire HBH header
            ip6Header.SetNextHeader(hbh.GetNextHeader());
        }
        else
        {
            // update HBH header length
            hbh.SetLength(hbh.GetLength() - 1);
            msg.Write(sizeof(ip6Header), sizeof(hbh), &hbh);
        }

        ip6Header.SetPayloadLength(ip6Header.GetPayloadLength() - sizeof(buf));
        msg.Write(0, sizeof(ip6Header), &ip6Header);
    }
    else if (mplOffset != 0)
    {
        // replace MPL Option with PadN Option
        OptionPadN padOption;

        padOption.Init(sizeof(OptionHeader) + mplLength);
        msg.Write(mplOffset, padOption.GetTotalLength(), &padOption);
    }

exit:
    return error;
}