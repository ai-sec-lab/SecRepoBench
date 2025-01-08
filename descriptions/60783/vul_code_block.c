tatic int flv_same_video_codec(AVFormatContext *s, AVCodecParameters *vpar, int flags)
{
    int flv_codecid = flags & FLV_VIDEO_CODECID_MASK;
    FLVContext *flv = s->priv_data;

    if (!vpar->codec_id && !vpar->codec_tag)
        return 1;

    if (flv->exheader) {
        uint8_t *codec_id_str = (uint8_t *)s->pb->buf_ptr;
        uint32_t codec_id = codec_id_str[3] | codec_id_str[2] << 8 | codec_id_str[1] << 16 | codec_id_str[0] << 24;
        switch(codec_id) {
            case MKBETAG('h', 'v', 'c', '1'):
                return vpar->codec_id == AV_CODEC_ID_HEVC;
            case MKBETAG('a', 'v', '0', '1'):
                return vpar->codec_id == AV_CODEC_ID_AV1;
            case MKBETAG('v', 'p', '0', '9'):
                return vpar->codec_id == AV_CODEC_ID_VP9;
            default:
                break;
        }
    }

    switch (flv_codecid) {
    case FLV_CODECID_H263:
        return vpar->codec_id == AV_CODEC_ID_FLV1;
    case FLV_CODECID_SCREEN:
        return vpar->codec_id == AV_CODEC_ID_FLASHSV;
    case FLV_CODECID_SCREEN2:
        return vpar->codec_id == AV_CODEC_ID_FLASHSV2;
    case FLV_CODECID_VP6:
        return vpar->codec_id == AV_CODEC_ID_VP6F;
    case FLV_CODECID_VP6A:
        return vpar->codec_id == AV_CODEC_ID_VP6A;
    case FLV_CODECID_H264:
        return vpar->codec_id == AV_CODEC_ID_H264;
    default:
        return vpar->codec_tag == flv_codecid;
    }
