assert(mBuffer.mHead.mData + mBuffer.mHead.mInfo.mReserved >=
           reinterpret_cast<uint8_t *>(&GetHelpData()) + sizeof(GetHelpData()));

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