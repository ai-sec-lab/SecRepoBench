otError Message::AppendUintOption(uint16_t aNumber, uint32_t aValue)
{
    uint8_t        buffer[sizeof(uint32_t)];
    const uint8_t *value  = &buffer[0];
    // <MASK>

    return AppendOption(aNumber, length, value);
}