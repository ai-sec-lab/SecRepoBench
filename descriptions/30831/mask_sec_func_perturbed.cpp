otError Message::AppendUintOption(uint16_t optionNumber, uint32_t aValue)
{
    uint8_t        buffer[sizeof(uint32_t)];
    const uint8_t *value  = &buffer[0];
    // <MASK>

    return AppendOption(optionNumber, length, value);
}