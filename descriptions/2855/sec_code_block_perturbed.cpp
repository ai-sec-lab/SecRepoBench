VerifyOrExit(kTokenOffset <= length);
    aMessage.Read(offset, kTokenOffset, mHeader.mBytes);
    mHeaderLength = kTokenOffset;
    offset += kTokenOffset;

    VerifyOrExit(GetVersion() == 1);

    tokenLength = GetTokenLength();
    VerifyOrExit(tokenLength <= kMaxTokenLength && (mHeaderLength + tokenLength) <= length);
    aMessage.Read(offset, tokenLength, mHeader.mBytes + mHeaderLength);
    mHeaderLength += tokenLength;
    offset += tokenLength;

    while (mHeaderLength < length)
    {
        VerifyOrExit(mHeaderLength + kMaxOptionHeaderSize <= kMaxHeaderLength);

        aMessage.Read(offset, kMaxOptionHeaderSize, mHeader.mBytes + mHeaderLength);

        if (mHeader.mBytes[mHeaderLength] == 0xff)
        {
            mHeaderLength += sizeof(uint8_t);
            // RFC7252: The presence of a marker followed by a zero-length payload MUST be processed
            // as a message format error.
            VerifyOrExit(mHeaderLength < length);
            ExitNow(error = OT_ERROR_NONE);
        }

        if (firstOption)
        {
            mFirstOptionOffset = mHeaderLength;
        }

        optionDifference = mHeader.mBytes[mHeaderLength] >> 4;
        optionLength = mHeader.mBytes[mHeaderLength] & 0xf;
        mHeaderLength += sizeof(uint8_t);
        offset += sizeof(uint8_t);

        if (optionDifference < kOption1ByteExtension)
        {
            // do nothing
        }
        else if (optionDifference == kOption1ByteExtension)
        {
            optionDifference = kOption1ByteExtensionOffset + mHeader.mBytes[mHeaderLength];
            mHeaderLength += sizeof(uint8_t);
            offset += sizeof(uint8_t);
        }
        else if (optionDifference == kOption2ByteExtension)
        {
            optionDifference = kOption2ByteExtensionOffset +
                          static_cast<uint16_t>((mHeader.mBytes[mHeaderLength] << 8) |
                                                (mHeader.mBytes[mHeaderLength + 1]));
            mHeaderLength += sizeof(uint16_t);
            offset += sizeof(uint16_t);
        }
        else
        {
            ExitNow(error = OT_ERROR_PARSE);
        }

        if (optionLength < kOption1ByteExtension)
        {
            // do nothing
        }
        else if (optionLength == kOption1ByteExtension)
        {
            optionLength = kOption1ByteExtensionOffset + mHeader.mBytes[mHeaderLength];
            mHeaderLength += sizeof(uint8_t);
            offset += sizeof(uint8_t);
        }
        else if (optionLength == kOption2ByteExtension)
        {
            optionLength = kOption2ByteExtensionOffset +
                           static_cast<uint16_t>((mHeader.mBytes[mHeaderLength] << 8) |
                                                 (mHeader.mBytes[mHeaderLength + 1]));
            mHeaderLength += sizeof(uint16_t);
            offset += sizeof(uint16_t);
        }
        else
        {
            ExitNow(error = OT_ERROR_PARSE);
        }

        if (firstOption)
        {
            mOption.mNumber = optionDifference;
            mOption.mLength = optionLength;
            mOption.mValue = mHeader.mBytes + mHeaderLength;
            mNextOptionOffset = mHeaderLength + optionLength;
            firstOption = false;
        }

        VerifyOrExit(mHeaderLength + optionLength <= kMaxHeaderLength);
        VerifyOrExit(mHeaderLength + optionLength <= length);

        aMessage.Read(offset, optionLength, mHeader.mBytes + mHeaderLength);
        mHeaderLength += static_cast<uint8_t>(optionLength);
        offset += optionLength;
    }

    if (mHeaderLength == length)
    {
        // No payload present - return success.
        error = OT_ERROR_NONE;
    }