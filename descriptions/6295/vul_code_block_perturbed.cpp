if (!is_valid_endian_marker(data, &isLittleEndian)) {
        return false;
    }

    // Get the offset from the start of the marker.
    // Though this only reads four bytes, use a larger int in case it overflows.
    uint64_t offset = get_endian_int(data + 4, isLittleEndian);

    // Require that the marker is at least large enough to contain the number of entries.
    if (data_length < offset + 2) {
        return false;
    }
    uint32_t numEntries = get_endian_short(data + offset, isLittleEndian);

    // Tag (2 bytes), Datatype (2 bytes), Number of elements (4 bytes), Data (4 bytes)
    const uint32_t kEntrySize = 12;
    const auto max = SkTo<uint32_t>((data_length - offset - 2) / kEntrySize);
    numEntries = SkTMin(numEntries, max);

    // Advance the data to the start of the entries.
    data += offset + 2;

    const uint16_t kOriginTag = 0x112;
    const uint16_t kOriginType = 3;
    for (uint32_t i = 0; i < numEntries; i++, data += kEntrySize) {
        uint16_t tag = get_endian_short(data, isLittleEndian);
        uint16_t type = get_endian_short(data + 2, isLittleEndian);
        uint32_t count = get_endian_int(data + 4, isLittleEndian);
        if (kOriginTag == tag && kOriginType == type && 1 == count) {
            uint16_t val = get_endian_short(data + 8, isLittleEndian);
            if (0 < val && val <= kLast_SkEncodedOrigin) {
                *orientation = (SkEncodedOrigin) val;
                return true;
            }
        }
    }

    return false;