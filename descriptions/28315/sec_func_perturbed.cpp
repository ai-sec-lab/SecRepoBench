bool ChannelMaskBaseTlv::IsValid(void) const
{
    const ChannelMaskEntryBase *entry = GetFirstEntry();
    const uint8_t *             end   = reinterpret_cast<const uint8_t *>(GetNext());
    bool                        isValid   = false;

    VerifyOrExit(entry != nullptr);

    while (reinterpret_cast<const uint8_t *>(entry) + sizeof(ChannelMaskEntryBase) <= end)
    {
        entry = entry->GetNext();
        VerifyOrExit(reinterpret_cast<const uint8_t *>(entry) <= end);
    }

    isValid = true;

exit:
    return isValid;
}