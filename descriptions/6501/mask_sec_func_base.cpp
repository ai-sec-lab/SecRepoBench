size_t SkPath::readFromMemory_EQ4(const void* storage, size_t length) {
    SkRBuffer buffer(storage, length);
    uint32_t packed;
    if (!buffer.readU32(&packed)) {
        return 0;
    }

    SkASSERT(extract_version(packed) == 4);

    switch (extract_serializationtype(packed)) {
        case SerializationType::kRRect:
            return this->readAsRRect(storage, length);
        case SerializationType::kGeneral:
            break;  // fall through
        default:
            return 0;
    }

    int32_t pts, cnx, vbs;
    if (!buffer.readS32(&pts) || !buffer.readS32(&cnx) || !buffer.readS32(&vbs)) {
        return 0;
    }

    const SkPoint* points = buffer.skipCount<SkPoint>(pts);
    const SkScalar* conics = buffer.skipCount<SkScalar>(cnx);
    const uint8_t* verbs = buffer.skipCount<uint8_t>(vbs);
    buffer.skipToAlign4();
    if (!buffer.isValid()) {
        return 0;
    }
    SkASSERT(buffer.pos() <= length);

// <MASK>

    *this = std::move(tmp);
    return buffer.pos();
}