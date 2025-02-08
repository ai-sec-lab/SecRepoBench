bool ChannelMaskBaseTlv::IsValid(void) const
{
    const ChannelMaskEntryBase *entry = GetFirstEntry();
    const uint8_t *             end   = reinterpret_cast<const uint8_t *>(GetNext());
    // <MASK>

    ret = true;

exit:
    return ret;
}