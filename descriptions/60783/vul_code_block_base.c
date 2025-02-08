
retry:
    /* pkt size is repeated at end. skip it */
    pos  = avio_tell(s->pb);
    type = (avio_r8(s->pb) & 0x1F);
    orig_size =
    size = avio_rb24(s->pb);
    flv->sum_flv_tag_size += size + 11LL;
    dts  = avio_rb24(s->pb);
    dts |= (unsigned)avio_r8(s->pb) << 24;
    av_log(s, AV_LOG_TRACE, "type:%d, size:%d, last:%d, dts:%"PRId64" pos:%"PRId64"\n", type, size, last, dts, avio_tell(s->pb));
    if (avio_feof(s->pb))
        return AVERROR_EOF;
    avio_skip(s->pb, 3); /* stream id, always 0 */
    flags = 0;

    if (flv->validate_next < flv->validate_count) {
        int64_t validate_pos = flv->validate_index[flv->validate_next].pos;
        if (pos == validate_pos) {
            if (FFABS(dts - flv->validate_index[flv->validate_next].dts) <=
                VALIDATE_INDEX_TS_THRESH) {
                flv->validate_next++;
            } else {
                clear_index_entries(s, validate_pos);
                flv->validate_count = 0;
            }
        } else if (pos > validate_pos) {
            clear_index_entries(s, validate_pos);
            flv->validate_count = 0;
        }
    }

    if (size == 0) {
        ret = FFERROR_REDO;
        goto leave;
    }

    next = size + avio_tell(s->pb);

    if (type == FLV_TAG_TYPE_AUDIO) {
        stream_type = FLV_STREAM_TYPE_AUDIO;
        flags    = avio_r8(s->pb);
        size--;
    } else if (type == FLV_TAG_TYPE_VIDEO) {
        stream_type = FLV_STREAM_TYPE_VIDEO;
        flags    = avio_r8(s->pb);
        /*
         * Reference Enhancing FLV 2023-03-v1.0.0-B.8
         * https://github.com/veovera/enhanced-rtmp/blob/main/enhanced-rtmp-v1.pdf
         * */
        flv->exheader = (flags >> 7) & 1;
        size--;
        if ((flags & FLV_VIDEO_FRAMETYPE_MASK) == FLV_FRAME_VIDEO_INFO_CMD)
            goto skip;
    } else if (type == FLV_TAG_TYPE_META) {
        stream_type=FLV_STREAM_TYPE_SUBTITLE;
        if (size > 13 + 1 + 4) { // Header-type metadata stuff
            int type;
            meta_pos = avio_tell(s->pb);
            type = flv_read_metabody(s, next);
            if (type == 0 && dts == 0 || type < 0) {
                if (type < 0 && flv->validate_count &&
                    flv->validate_index[0].pos     > next &&
                    flv->validate_index[0].pos - 4 < next) {
                    av_log(s, AV_LOG_WARNING, "Adjusting next position due to index mismatch\n");
                    next = flv->validate_index[0].pos - 4;
                }
                goto skip;
            } else if (type == TYPE_ONTEXTDATA) {
                avpriv_request_sample(s, "OnTextData packet");
                return flv_data_packet(s, pkt, dts, next);
            } else if (type == TYPE_ONCAPTION) {
                return flv_data_packet(s, pkt, dts, next);
            } else if (type == TYPE_UNKNOWN) {
                stream_type = FLV_STREAM_TYPE_DATA;
            }
            avio_seek(s->pb, meta_pos, SEEK_SET);
        }
    } else {
        av_log(s, AV_LOG_DEBUG,
               "Skipping flv packet: type %d, size %d, flags %d.\n",
               type, size, flags);
skip:
        if (avio_seek(s->pb, next, SEEK_SET) != next) {
            // This can happen if flv_read_metabody above read past
            // next, on a non-seekable input, and the preceding data has
            // been flushed out from the IO buffer.
            av_log(s, AV_LOG_ERROR, "Unable to seek to the next packet\n");
            return AVERROR_INVALIDDATA;
        }
        ret = FFERROR_REDO;
        goto leave;
    }

    /* skip empty data packets */
    if (!size) {
        ret = FFERROR_REDO;
        goto leave;
    }

    /* now find stream */
    for (i = 0; i < s->nb_streams; i++) {
        st = s->streams[i];
        if (stream_type == FLV_STREAM_TYPE_AUDIO) {
            if (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO &&
                (s->audio_codec_id || flv_same_audio_codec(st->codecpar, flags)))
                break;
        } else if (stream_type == FLV_STREAM_TYPE_VIDEO) {
            if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO &&
                (s->video_codec_id || flv_same_video_codec(s, st->codecpar, flags)))
                break;
        } else if (stream_type == FLV_STREAM_TYPE_SUBTITLE) {
            if (st->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE)
                break;
        } else if (stream_type == FLV_STREAM_TYPE_DATA) {
            if (st->codecpar->codec_type == AVMEDIA_TYPE_DATA)
                break;
        }
    }
    if (i == s->nb_streams) {
        static const enum AVMediaType stream_types[] = {AVMEDIA_TYPE_VIDEO, AVMEDIA_TYPE_AUDIO, AVMEDIA_TYPE_SUBTITLE, AVMEDIA_TYPE_DATA};
        st = create_stream(s, stream_types[stream_type]);
        if (!st)
            return AVERROR(ENOMEM);
    }
    av_log(s, AV_LOG_TRACE, "%d %X %d \n", stream_type, flags, st->discard);

    if (flv->time_pos <= pos) {
        dts += flv->time_offset;
    }

    if ((s->pb->seekable & AVIO_SEEKABLE_NORMAL) &&
        ((flags & FLV_VIDEO_FRAMETYPE_MASK) == FLV_FRAME_KEY ||
         stream_type == FLV_STREAM_TYPE_AUDIO))
        av_add_index_entry(st, pos, dts, size, 0, AVINDEX_KEYFRAME);

    if ((st->discard >= AVDISCARD_NONKEY && !((flags & FLV_VIDEO_FRAMETYPE_MASK) == FLV_FRAME_KEY || stream_type == FLV_STREAM_TYPE_AUDIO)) ||
        (st->discard >= AVDISCARD_BIDIR && ((flags & FLV_VIDEO_FRAMETYPE_MASK) == FLV_FRAME_DISP_INTER && stream_type == FLV_STREAM_TYPE_VIDEO)) ||
         st->discard >= AVDISCARD_ALL) {
        avio_seek(s->pb, next, SEEK_SET);
        ret = FFERROR_REDO;
        goto leave;
    }

    // if not streamed and no duration from metadata then seek to end to find
    // the duration from the timestamps
    if ((s->pb->seekable & AVIO_SEEKABLE_NORMAL) &&
        (!s->duration || s->duration == AV_NOPTS_VALUE) &&
        !flv->searched_for_end) {
        int size;
        const int64_t pos   = avio_tell(s->pb);
        // Read the last 4 bytes of the file, this should be the size of the
        // previous FLV tag. Use the timestamp of its payload as duration.
        int64_t fsize       = avio_size(s->pb);
retry_duration:
        avio_seek(s->pb, fsize - 4, SEEK_SET);
        size = avio_rb32(s->pb);
        if (size > 0 && size < fsize) {
            // Seek to the start of the last FLV tag at position (fsize - 4 - size)
            // but skip the byte indicating the type.
            avio_seek(s->pb, fsize - 3 - size, SEEK_SET);
            if (size == avio_rb24(s->pb) + 11) {
                uint32_t ts = avio_rb24(s->pb);
                ts         |= (unsigned)avio_r8(s->pb) << 24;
                if (ts)
                    s->duration = ts * (int64_t)AV_TIME_BASE / 1000;
                else if (fsize >= 8 && fsize - 8 >= size) {
                    fsize -= size+4;
                    goto retry_duration;
                }
            }
        }

        avio_seek(s->pb, pos, SEEK_SET);
        flv->searched_for_end = 1;
    }

    if (stream_type == FLV_STREAM_TYPE_AUDIO) {
        int bits_per_coded_sample;
        channels = (flags & FLV_AUDIO_CHANNEL_MASK) == FLV_STEREO ? 2 : 1;
        sample_rate = 44100 << ((flags & FLV_AUDIO_SAMPLERATE_MASK) >>
                                FLV_AUDIO_SAMPLERATE_OFFSET) >> 3;
        bits_per_coded_sample = (flags & FLV_AUDIO_SAMPLESIZE_MASK) ? 16 : 8;
        if (!av_channel_layout_check(&st->codecpar->ch_layout) ||
            !st->codecpar->sample_rate ||
            !st->codecpar->bits_per_coded_sample) {
            av_channel_layout_default(&st->codecpar->ch_layout, channels);
            st->codecpar->sample_rate           = sample_rate;
            st->codecpar->bits_per_coded_sample = bits_per_coded_sample;
        }
        if (!st->codecpar->codec_id) {
            flv_set_audio_codec(s, st, st->codecpar,
                                flags & FLV_AUDIO_CODECID_MASK);
            flv->last_sample_rate =
            sample_rate           = st->codecpar->sample_rate;
            flv->last_channels    =
            channels              = st->codecpar->ch_layout.nb_channels;
        } else {
            AVCodecParameters *par = avcodec_parameters_alloc();
            if (!par) {
                ret = AVERROR(ENOMEM);
                goto leave;
            }
            par->sample_rate = sample_rate;
            par->bits_per_coded_sample = bits_per_coded_sample;
            flv_set_audio_codec(s, st, par, flags & FLV_AUDIO_CODECID_MASK);
            sample_rate = par->sample_rate;
            avcodec_parameters_free(&par);
        }
    } else if (stream_type == FLV_STREAM_TYPE_VIDEO) {
        int ret = flv_set_video_codec(s, st, flags, 1);
        if (ret < 0)
            return ret;
        size -= ret;
    } else if (stream_type == FLV_STREAM_TYPE_SUBTITLE) {
        st->codecpar->codec_id = AV_CODEC_ID_TEXT;
    } else if (stream_type == FLV_STREAM_TYPE_DATA) {
        st->codecpar->codec_id = AV_CODEC_ID_NONE; // Opaque AMF data
    }