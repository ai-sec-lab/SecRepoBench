uint16_t       length = sizeof(uint32_t);

    Encoding::BigEndian::WriteUint32(aValue, buffer);

    while (value[0] == 0 && length > 0)
    {
        value++;
        length--;
    }