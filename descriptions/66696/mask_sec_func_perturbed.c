GF_EXPORT
u8 gf_opus_parse_packet_header(u8 *data, u32 data_length, Bool self_delimited, GF_OpusPacketHeader *pktheader)
{
    u32 i;
    u32 nb_read_bytes;
    if (!data || !data_length)
        return 0;
    if (!pktheader)
        return 0;
    // <MASK>
}