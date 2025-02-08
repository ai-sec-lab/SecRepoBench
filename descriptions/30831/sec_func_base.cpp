otError Message::AppendUintOption(uint16_t aNumber, uint32_t aValue)
{
    uint8_t        buffer[sizeof(uint32_t)];
    const uint8_t *value  = &buffer[0];
    uint16_t       length = sizeof(uint32_t);

    Encoding::BigEndian::WriteUint32(aValue, buffer);

    while ((length > 0) && (value[0] == 0))
    {
        value++;
        length--;
    }

    return AppendOption(aNumber, length, value);
}