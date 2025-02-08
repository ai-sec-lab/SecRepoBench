static int ea_read_packet(AVFormatContext *s, AVPacket *pkt)
{
    EaDemuxContext *ea = s->priv_data;
    AVIOContext *pb    = s->pb;
    int partial_packet = 0;
    // <MASK>

    return ret;
}