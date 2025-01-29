bool                        ret   = false;

    VerifyOrExit(entry != nullptr);

    while (reinterpret_cast<const uint8_t *>(entry) + sizeof(ChannelMaskEntryBase) <= end)
    {
        entry = entry->GetNext();
        VerifyOrExit(reinterpret_cast<const uint8_t *>(entry) <= end);
    }