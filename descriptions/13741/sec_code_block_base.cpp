uint16_t offset;
    uint16_t end;

    SuccessOrExit(GetValueOffset(aMessage, kChannelMask, offset, end));
    end += offset;

    while (offset + sizeof(ChannelMaskEntryBase) <= end)
    {
        ChannelMaskEntry entry;

        aMessage.Read(offset, sizeof(ChannelMaskEntryBase), &entry);
        VerifyOrExit(offset + entry.GetEntrySize() <= end);

        switch (entry.GetChannelPage())
        {
#if OPENTHREAD_CONFIG_RADIO_2P4GHZ_OQPSK_SUPPORT
        case OT_RADIO_CHANNEL_PAGE_0:
            aMessage.Read(offset, sizeof(entry), &entry);
            mask |= entry.GetMask() & OT_RADIO_2P4GHZ_OQPSK_CHANNEL_MASK;
            break;
#endif

#if OPENTHREAD_CONFIG_RADIO_915MHZ_OQPSK_SUPPORT
        case OT_RADIO_CHANNEL_PAGE_2:
            aMessage.Read(offset, sizeof(entry), &entry);
            mask |= entry.GetMask() & OT_RADIO_915MHZ_OQPSK_CHANNEL_MASK;
            break;
#endif
        }

        offset += entry.GetEntrySize();
    }