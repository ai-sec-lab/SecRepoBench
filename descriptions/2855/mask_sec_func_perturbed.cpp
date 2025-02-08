otError Header::FromMessage(const Message &aMessage, uint16_t aMetadataSize)
{
    otError error = OT_ERROR_PARSE;
    uint16_t offset = aMessage.GetOffset();
    uint16_t length = aMessage.GetLength() - aMessage.GetOffset();
    uint8_t tokenLength;
    bool firstOption = true;
    uint16_t optionDifference;
    uint16_t optionLength;

    length -= aMetadataSize;

    Init();

    // <MASK>

exit:

    // In case any step failed, prevent access to corrupt Option
    if (error != OT_ERROR_NONE)
    {
        mFirstOptionOffset = 0;
    }

    return error;
}