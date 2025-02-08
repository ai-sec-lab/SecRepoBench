otError Message::ParseHeader(void)
{
    otError error = OT_ERROR_NONE;

    assert(mBuffer.mHead.mInfo.mReserved >=
           sizeof(GetHelpData()) +
               static_cast<size_t>((reinterpret_cast<uint8_t *>(&GetHelpData()) - mBuffer.mHead.mData)));

    memset(&GetHelpData(), 0, sizeof(GetHelpData()));

    GetHelpData().mHeaderOffset = GetOffset();
    Read(GetHelpData().mHeaderOffset, sizeof(GetHelpData().mHeader), &GetHelpData().mHeader);

    for (const otCoapOption *option = GetFirstOption(); option != NULL; option = GetNextOption())
    {
    }

    VerifyOrExit(GetHelpData().mNextOptionOffset > 0, error = OT_ERROR_PARSE);
    GetHelpData().mHeaderLength = GetHelpData().mNextOptionOffset - GetHelpData().mHeaderOffset;
    MoveOffset(GetHelpData().mHeaderLength);

exit:
    return error;
}