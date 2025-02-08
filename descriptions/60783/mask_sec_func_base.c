static int flv_read_packet(AVFormatContext *s, AVPacket *pkt)
{
    FLVContext *flv = s->priv_data;
    int ret, i, size, flags;
    enum FlvTagType type;
    int stream_type=-1;
    int64_t next, pos, meta_pos;
    int64_t dts, pts = AV_NOPTS_VALUE;
    int av_uninit(channels);
    int av_uninit(sample_rate);
    AVStream *st    = NULL;
    int last = -1;
    int orig_size;
    // <MASK>

    if (st->codecpar->codec_id == AV_CODEC_ID_AAC ||
        st->codecpar->codec_id == AV_CODEC_ID_H264 ||
        st->codecpar->codec_id == AV_CODEC_ID_MPEG4 ||
        st->codecpar->codec_id == AV_CODEC_ID_HEVC ||
        st->codecpar->codec_id == AV_CODEC_ID_AV1 ||
        st->codecpar->codec_id == AV_CODEC_ID_VP9) {
        int type = 0;
        if (flv->exheader && stream_type == FLV_STREAM_TYPE_VIDEO) {
            type = flags & 0x0F;
        } else {
            type = avio_r8(s->pb);
            size--;
        }

        if (size < 0) {
            ret = AVERROR_INVALIDDATA;
            goto leave;
        }

        if (st->codecpar->codec_id == AV_CODEC_ID_H264 || st->codecpar->codec_id == AV_CODEC_ID_MPEG4 ||
            (st->codecpar->codec_id == AV_CODEC_ID_HEVC && type == PacketTypeCodedFrames)) {
            // sign extension
            int32_t cts = (avio_rb24(s->pb) + 0xff800000) ^ 0xff800000;
            pts = av_sat_add64(dts, cts);
            if (cts < 0) { // dts might be wrong
                if (!flv->wrong_dts)
                    av_log(s, AV_LOG_WARNING,
                        "Negative cts, previous timestamps might be wrong.\n");
                flv->wrong_dts = 1;
            } else if (FFABS(dts - pts) > 1000*60*15) {
                av_log(s, AV_LOG_WARNING,
                       "invalid timestamps %"PRId64" %"PRId64"\n", dts, pts);
                dts = pts = AV_NOPTS_VALUE;
            }
            size -= 3;
        }
        if (type == 0 && (!st->codecpar->extradata || st->codecpar->codec_id == AV_CODEC_ID_AAC ||
            st->codecpar->codec_id == AV_CODEC_ID_H264 || st->codecpar->codec_id == AV_CODEC_ID_HEVC ||
            st->codecpar->codec_id == AV_CODEC_ID_AV1 || st->codecpar->codec_id == AV_CODEC_ID_VP9)) {
            AVDictionaryEntry *t;

            if (st->codecpar->extradata) {
                if ((ret = flv_queue_extradata(flv, s->pb, stream_type, size)) < 0)
                    return ret;
                ret = FFERROR_REDO;
                goto leave;
            }
            if ((ret = flv_get_extradata(s, st, size)) < 0)
                return ret;

            /* Workaround for buggy Omnia A/XE encoder */
            t = av_dict_get(s->metadata, "Encoder", NULL, 0);
            if (st->codecpar->codec_id == AV_CODEC_ID_AAC && t && !strcmp(t->value, "Omnia A/XE"))
                st->codecpar->extradata_size = 2;

            ret = FFERROR_REDO;
            goto leave;
        }
    }

    /* skip empty data packets */
    if (!size) {
        ret = FFERROR_REDO;
        goto leave;
    }

    ret = av_get_packet(s->pb, pkt, size);
    if (ret < 0)
        return ret;
    pkt->dts          = dts;
    pkt->pts          = pts == AV_NOPTS_VALUE ? dts : pts;
    pkt->stream_index = st->index;
    pkt->pos          = pos;
    if (flv->new_extradata[stream_type]) {
        int ret = av_packet_add_side_data(pkt, AV_PKT_DATA_NEW_EXTRADATA,
                                          flv->new_extradata[stream_type],
                                          flv->new_extradata_size[stream_type]);
        if (ret >= 0) {
            flv->new_extradata[stream_type]      = NULL;
            flv->new_extradata_size[stream_type] = 0;
        }
    }
    if (stream_type == FLV_STREAM_TYPE_AUDIO &&
                    (sample_rate != flv->last_sample_rate ||
                     channels    != flv->last_channels)) {
        flv->last_sample_rate = sample_rate;
        flv->last_channels    = channels;
        ff_add_param_change(pkt, channels, 0, sample_rate, 0, 0);
    }

    if (stream_type == FLV_STREAM_TYPE_AUDIO ||
        (flags & FLV_VIDEO_FRAMETYPE_MASK) == FLV_FRAME_KEY ||
        stream_type == FLV_STREAM_TYPE_SUBTITLE ||
        stream_type == FLV_STREAM_TYPE_DATA)
        pkt->flags |= AV_PKT_FLAG_KEY;

leave:
    last = avio_rb32(s->pb);
    if (!flv->trust_datasize) {
        if (last != orig_size + 11 && last != orig_size + 10 &&
            !avio_feof(s->pb) &&
            (last != orig_size || !last) && last != flv->sum_flv_tag_size &&
            !flv->broken_sizes) {
            av_log(s, AV_LOG_ERROR, "Packet mismatch %d %d %"PRId64"\n", last, orig_size + 11, flv->sum_flv_tag_size);
            avio_seek(s->pb, pos + 1, SEEK_SET);
            ret = resync(s);
            av_packet_unref(pkt);
            if (ret >= 0) {
                goto retry;
            }
        }
    }

    if (ret >= 0)
        flv->last_ts = pkt->dts;

    return ret;
}