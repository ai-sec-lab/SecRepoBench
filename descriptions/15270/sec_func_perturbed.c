int attribute_align_arg avcodec_open2(AVCodecContext *codeccontext, const AVCodec *codec, AVDictionary **options)
{
    int ret = 0;
    AVDictionary *tmp = NULL;
    const AVPixFmtDescriptor *pixdesc;

    if (avcodec_is_open(codeccontext))
        return 0;

    if ((!codec && !codeccontext->codec)) {
        av_log(codeccontext, AV_LOG_ERROR, "No codec provided to avcodec_open2()\n");
        return AVERROR(EINVAL);
    }
    if ((codec && codeccontext->codec && codec != codeccontext->codec)) {
        av_log(codeccontext, AV_LOG_ERROR, "This AVCodecContext was allocated for %s, "
                                    "but %s passed to avcodec_open2()\n", codeccontext->codec->name, codec->name);
        return AVERROR(EINVAL);
    }
    if (!codec)
        codec = codeccontext->codec;

    if (codeccontext->extradata_size < 0 || codeccontext->extradata_size >= FF_MAX_EXTRADATA_SIZE)
        return AVERROR(EINVAL);

    if (options)
        av_dict_copy(&tmp, *options, 0);

    ff_lock_avcodec(codeccontext, codec);

    codeccontext->internal = av_mallocz(sizeof(*codeccontext->internal));
    if (!codeccontext->internal) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    codeccontext->internal->pool = av_mallocz(sizeof(*codeccontext->internal->pool));
    if (!codeccontext->internal->pool) {
        ret = AVERROR(ENOMEM);
        goto free_and_end;
    }

    codeccontext->internal->to_free = av_frame_alloc();
    if (!codeccontext->internal->to_free) {
        ret = AVERROR(ENOMEM);
        goto free_and_end;
    }

    codeccontext->internal->compat_decode_frame = av_frame_alloc();
    if (!codeccontext->internal->compat_decode_frame) {
        ret = AVERROR(ENOMEM);
        goto free_and_end;
    }

    codeccontext->internal->buffer_frame = av_frame_alloc();
    if (!codeccontext->internal->buffer_frame) {
        ret = AVERROR(ENOMEM);
        goto free_and_end;
    }

    codeccontext->internal->buffer_pkt = av_packet_alloc();
    if (!codeccontext->internal->buffer_pkt) {
        ret = AVERROR(ENOMEM);
        goto free_and_end;
    }

    codeccontext->internal->ds.in_pkt = av_packet_alloc();
    if (!codeccontext->internal->ds.in_pkt) {
        ret = AVERROR(ENOMEM);
        goto free_and_end;
    }

    codeccontext->internal->last_pkt_props = av_packet_alloc();
    if (!codeccontext->internal->last_pkt_props) {
        ret = AVERROR(ENOMEM);
        goto free_and_end;
    }

    codeccontext->internal->skip_samples_multiplier = 1;

    if (codec->priv_data_size > 0) {
        if (!codeccontext->priv_data) {
            codeccontext->priv_data = av_mallocz(codec->priv_data_size);
            if (!codeccontext->priv_data) {
                ret = AVERROR(ENOMEM);
                goto end;
            }
            if (codec->priv_class) {
                *(const AVClass **)codeccontext->priv_data = codec->priv_class;
                av_opt_set_defaults(codeccontext->priv_data);
            }
        }
        if (codec->priv_class && (ret = av_opt_set_dict(codeccontext->priv_data, &tmp)) < 0)
            goto free_and_end;
    } else {
        codeccontext->priv_data = NULL;
    }
    if ((ret = av_opt_set_dict(codeccontext, &tmp)) < 0)
        goto free_and_end;

    if (codeccontext->codec_whitelist && av_match_list(codec->name, codeccontext->codec_whitelist, ',') <= 0) {
        av_log(codeccontext, AV_LOG_ERROR, "Codec (%s) not on whitelist \'%s\'\n", codec->name, codeccontext->codec_whitelist);
        ret = AVERROR(EINVAL);
        goto free_and_end;
    }

    // only call ff_set_dimensions() for non H.264/VP6F/DXV codecs so as not to overwrite previously setup dimensions
    if (!(codeccontext->coded_width && codeccontext->coded_height && codeccontext->width && codeccontext->height &&
          (codeccontext->codec_id == AV_CODEC_ID_H264 || codeccontext->codec_id == AV_CODEC_ID_VP6F || codeccontext->codec_id == AV_CODEC_ID_DXV))) {
    if (codeccontext->coded_width && codeccontext->coded_height)
        ret = ff_set_dimensions(codeccontext, codeccontext->coded_width, codeccontext->coded_height);
    else if (codeccontext->width && codeccontext->height)
        ret = ff_set_dimensions(codeccontext, codeccontext->width, codeccontext->height);
    if (ret < 0)
        goto free_and_end;
    }

    if ((codeccontext->coded_width || codeccontext->coded_height || codeccontext->width || codeccontext->height)
        && (  av_image_check_size2(codeccontext->coded_width, codeccontext->coded_height, codeccontext->max_pixels, AV_PIX_FMT_NONE, 0, codeccontext) < 0
           || av_image_check_size2(codeccontext->width,       codeccontext->height,       codeccontext->max_pixels, AV_PIX_FMT_NONE, 0, codeccontext) < 0)) {
        av_log(codeccontext, AV_LOG_WARNING, "Ignoring invalid width/height values\n");
        ff_set_dimensions(codeccontext, 0, 0);
    }

    if (codeccontext->width > 0 && codeccontext->height > 0) {
        if (av_image_check_sar(codeccontext->width, codeccontext->height,
                               codeccontext->sample_aspect_ratio) < 0) {
            av_log(codeccontext, AV_LOG_WARNING, "ignoring invalid SAR: %u/%u\n",
                   codeccontext->sample_aspect_ratio.num,
                   codeccontext->sample_aspect_ratio.den);
            codeccontext->sample_aspect_ratio = (AVRational){ 0, 1 };
        }
    }

    /* if the decoder init function was already called previously,
     * free the already allocated subtitle_header before overwriting it */
    if (av_codec_is_decoder(codec))
        av_freep(&codeccontext->subtitle_header);

    if (codeccontext->channels > FF_SANE_NB_CHANNELS) {
        av_log(codeccontext, AV_LOG_ERROR, "Too many channels: %d\n", codeccontext->channels);
        ret = AVERROR(EINVAL);
        goto free_and_end;
    }

    codeccontext->codec = codec;
    if ((codeccontext->codec_type == AVMEDIA_TYPE_UNKNOWN || codeccontext->codec_type == codec->type) &&
        codeccontext->codec_id == AV_CODEC_ID_NONE) {
        codeccontext->codec_type = codec->type;
        codeccontext->codec_id   = codec->id;
    }
    if (codeccontext->codec_id != codec->id || (codeccontext->codec_type != codec->type
                                         && codeccontext->codec_type != AVMEDIA_TYPE_ATTACHMENT)) {
        av_log(codeccontext, AV_LOG_ERROR, "Codec type or id mismatches\n");
        ret = AVERROR(EINVAL);
        goto free_and_end;
    }
    codeccontext->frame_number = 0;
    codeccontext->codec_descriptor = avcodec_descriptor_get(codeccontext->codec_id);

    if ((codeccontext->codec->capabilities & AV_CODEC_CAP_EXPERIMENTAL) &&
        codeccontext->strict_std_compliance > FF_COMPLIANCE_EXPERIMENTAL) {
        const char *codec_string = av_codec_is_encoder(codec) ? "encoder" : "decoder";
        AVCodec *codec2;
        av_log(codeccontext, AV_LOG_ERROR,
               "The %s '%s' is experimental but experimental codecs are not enabled, "
               "add '-strict %d' if you want to use it.\n",
               codec_string, codec->name, FF_COMPLIANCE_EXPERIMENTAL);
        codec2 = av_codec_is_encoder(codec) ? avcodec_find_encoder(codec->id) : avcodec_find_decoder(codec->id);
        if (!(codec2->capabilities & AV_CODEC_CAP_EXPERIMENTAL))
            av_log(codeccontext, AV_LOG_ERROR, "Alternatively use the non experimental %s '%s'.\n",
                codec_string, codec2->name);
        ret = AVERROR_EXPERIMENTAL;
        goto free_and_end;
    }

    if (codeccontext->codec_type == AVMEDIA_TYPE_AUDIO &&
        (!codeccontext->time_base.num || !codeccontext->time_base.den)) {
        codeccontext->time_base.num = 1;
        codeccontext->time_base.den = codeccontext->sample_rate;
    }

    if (!HAVE_THREADS)
        av_log(codeccontext, AV_LOG_WARNING, "Warning: not compiled with thread support, using thread emulation\n");

    if (CONFIG_FRAME_THREAD_ENCODER && av_codec_is_encoder(codeccontext->codec)) {
        ff_unlock_avcodec(codec); //we will instantiate a few encoders thus kick the counter to prevent false detection of a problem
        ret = ff_frame_thread_encoder_init(codeccontext, options ? *options : NULL);
        ff_lock_avcodec(codeccontext, codec);
        if (ret < 0)
            goto free_and_end;
    }

    if (av_codec_is_decoder(codeccontext->codec)) {
        ret = ff_decode_bsfs_init(codeccontext);
        if (ret < 0)
            goto free_and_end;
    }

    if (HAVE_THREADS
        && !(codeccontext->internal->frame_thread_encoder && (codeccontext->active_thread_type&FF_THREAD_FRAME))) {
        ret = ff_thread_init(codeccontext);
        if (ret < 0) {
            goto free_and_end;
        }
    }
    if (!HAVE_THREADS && !(codec->capabilities & AV_CODEC_CAP_AUTO_THREADS))
        codeccontext->thread_count = 1;

    if (codeccontext->codec->max_lowres < codeccontext->lowres || codeccontext->lowres < 0) {
        av_log(codeccontext, AV_LOG_WARNING, "The maximum value for lowres supported by the decoder is %d\n",
               codeccontext->codec->max_lowres);
        codeccontext->lowres = codeccontext->codec->max_lowres;
    }

    if (av_codec_is_encoder(codeccontext->codec)) {
        int i;
#if FF_API_CODED_FRAME
FF_DISABLE_DEPRECATION_WARNINGS
        codeccontext->coded_frame = av_frame_alloc();
        if (!codeccontext->coded_frame) {
            ret = AVERROR(ENOMEM);
            goto free_and_end;
        }
FF_ENABLE_DEPRECATION_WARNINGS
#endif

        if (codeccontext->time_base.num <= 0 || codeccontext->time_base.den <= 0) {
            av_log(codeccontext, AV_LOG_ERROR, "The encoder timebase is not set.\n");
            ret = AVERROR(EINVAL);
            goto free_and_end;
        }

        if (codeccontext->codec->sample_fmts) {
            for (i = 0; codeccontext->codec->sample_fmts[i] != AV_SAMPLE_FMT_NONE; i++) {
                if (codeccontext->sample_fmt == codeccontext->codec->sample_fmts[i])
                    break;
                if (codeccontext->channels == 1 &&
                    av_get_planar_sample_fmt(codeccontext->sample_fmt) ==
                    av_get_planar_sample_fmt(codeccontext->codec->sample_fmts[i])) {
                    codeccontext->sample_fmt = codeccontext->codec->sample_fmts[i];
                    break;
                }
            }
            if (codeccontext->codec->sample_fmts[i] == AV_SAMPLE_FMT_NONE) {
                char buf[128];
                snprintf(buf, sizeof(buf), "%d", codeccontext->sample_fmt);
                av_log(codeccontext, AV_LOG_ERROR, "Specified sample format %s is invalid or not supported\n",
                       (char *)av_x_if_null(av_get_sample_fmt_name(codeccontext->sample_fmt), buf));
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
        }
        if (codeccontext->codec->pix_fmts) {
            for (i = 0; codeccontext->codec->pix_fmts[i] != AV_PIX_FMT_NONE; i++)
                if (codeccontext->pix_fmt == codeccontext->codec->pix_fmts[i])
                    break;
            if (codeccontext->codec->pix_fmts[i] == AV_PIX_FMT_NONE
                && !((codeccontext->codec_id == AV_CODEC_ID_MJPEG || codeccontext->codec_id == AV_CODEC_ID_LJPEG)
                     && codeccontext->strict_std_compliance <= FF_COMPLIANCE_UNOFFICIAL)) {
                char buf[128];
                snprintf(buf, sizeof(buf), "%d", codeccontext->pix_fmt);
                av_log(codeccontext, AV_LOG_ERROR, "Specified pixel format %s is invalid or not supported\n",
                       (char *)av_x_if_null(av_get_pix_fmt_name(codeccontext->pix_fmt), buf));
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
            if (codeccontext->codec->pix_fmts[i] == AV_PIX_FMT_YUVJ420P ||
                codeccontext->codec->pix_fmts[i] == AV_PIX_FMT_YUVJ411P ||
                codeccontext->codec->pix_fmts[i] == AV_PIX_FMT_YUVJ422P ||
                codeccontext->codec->pix_fmts[i] == AV_PIX_FMT_YUVJ440P ||
                codeccontext->codec->pix_fmts[i] == AV_PIX_FMT_YUVJ444P)
                codeccontext->color_range = AVCOL_RANGE_JPEG;
        }
        if (codeccontext->codec->supported_samplerates) {
            for (i = 0; codeccontext->codec->supported_samplerates[i] != 0; i++)
                if (codeccontext->sample_rate == codeccontext->codec->supported_samplerates[i])
                    break;
            if (codeccontext->codec->supported_samplerates[i] == 0) {
                av_log(codeccontext, AV_LOG_ERROR, "Specified sample rate %d is not supported\n",
                       codeccontext->sample_rate);
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
        }
        if (codeccontext->sample_rate < 0) {
            av_log(codeccontext, AV_LOG_ERROR, "Specified sample rate %d is not supported\n",
                    codeccontext->sample_rate);
            ret = AVERROR(EINVAL);
            goto free_and_end;
        }
        if (codeccontext->codec->channel_layouts) {
            if (!codeccontext->channel_layout) {
                av_log(codeccontext, AV_LOG_WARNING, "Channel layout not specified\n");
            } else {
                for (i = 0; codeccontext->codec->channel_layouts[i] != 0; i++)
                    if (codeccontext->channel_layout == codeccontext->codec->channel_layouts[i])
                        break;
                if (codeccontext->codec->channel_layouts[i] == 0) {
                    char buf[512];
                    av_get_channel_layout_string(buf, sizeof(buf), -1, codeccontext->channel_layout);
                    av_log(codeccontext, AV_LOG_ERROR, "Specified channel layout '%s' is not supported\n", buf);
                    ret = AVERROR(EINVAL);
                    goto free_and_end;
                }
            }
        }
        if (codeccontext->channel_layout && codeccontext->channels) {
            int channels = av_get_channel_layout_nb_channels(codeccontext->channel_layout);
            if (channels != codeccontext->channels) {
                char buf[512];
                av_get_channel_layout_string(buf, sizeof(buf), -1, codeccontext->channel_layout);
                av_log(codeccontext, AV_LOG_ERROR,
                       "Channel layout '%s' with %d channels does not match number of specified channels %d\n",
                       buf, channels, codeccontext->channels);
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
        } else if (codeccontext->channel_layout) {
            codeccontext->channels = av_get_channel_layout_nb_channels(codeccontext->channel_layout);
        }
        if (codeccontext->channels < 0) {
            av_log(codeccontext, AV_LOG_ERROR, "Specified number of channels %d is not supported\n",
                    codeccontext->channels);
            ret = AVERROR(EINVAL);
            goto free_and_end;
        }
        if(codeccontext->codec_type == AVMEDIA_TYPE_VIDEO) {
            pixdesc = av_pix_fmt_desc_get(codeccontext->pix_fmt);
            if (    codeccontext->bits_per_raw_sample < 0
                || (codeccontext->bits_per_raw_sample > 8 && pixdesc->comp[0].depth <= 8)) {
                av_log(codeccontext, AV_LOG_WARNING, "Specified bit depth %d not possible with the specified pixel formats depth %d\n",
                    codeccontext->bits_per_raw_sample, pixdesc->comp[0].depth);
                codeccontext->bits_per_raw_sample = pixdesc->comp[0].depth;
            }
            if (codeccontext->width <= 0 || codeccontext->height <= 0) {
                av_log(codeccontext, AV_LOG_ERROR, "dimensions not set\n");
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
        }
        if (   (codeccontext->codec_type == AVMEDIA_TYPE_VIDEO || codeccontext->codec_type == AVMEDIA_TYPE_AUDIO)
            && codeccontext->bit_rate>0 && codeccontext->bit_rate<1000) {
            av_log(codeccontext, AV_LOG_WARNING, "Bitrate %"PRId64" is extremely low, maybe you mean %"PRId64"k\n", codeccontext->bit_rate, codeccontext->bit_rate);
        }

        if (!codeccontext->rc_initial_buffer_occupancy)
            codeccontext->rc_initial_buffer_occupancy = codeccontext->rc_buffer_size * 3LL / 4;

        if (codeccontext->ticks_per_frame && codeccontext->time_base.num &&
            codeccontext->ticks_per_frame > INT_MAX / codeccontext->time_base.num) {
            av_log(codeccontext, AV_LOG_ERROR,
                   "ticks_per_frame %d too large for the timebase %d/%d.",
                   codeccontext->ticks_per_frame,
                   codeccontext->time_base.num,
                   codeccontext->time_base.den);
            goto free_and_end;
        }

        if (codeccontext->hw_frames_ctx) {
            AVHWFramesContext *frames_ctx = (AVHWFramesContext*)codeccontext->hw_frames_ctx->data;
            if (frames_ctx->format != codeccontext->pix_fmt) {
                av_log(codeccontext, AV_LOG_ERROR,
                       "Mismatching AVCodecContext.pix_fmt and AVHWFramesContext.format\n");
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
            if (codeccontext->sw_pix_fmt != AV_PIX_FMT_NONE &&
                codeccontext->sw_pix_fmt != frames_ctx->sw_format) {
                av_log(codeccontext, AV_LOG_ERROR,
                       "Mismatching AVCodecContext.sw_pix_fmt (%s) "
                       "and AVHWFramesContext.sw_format (%s)\n",
                       av_get_pix_fmt_name(codeccontext->sw_pix_fmt),
                       av_get_pix_fmt_name(frames_ctx->sw_format));
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
            codeccontext->sw_pix_fmt = frames_ctx->sw_format;
        }
    }

    codeccontext->pts_correction_num_faulty_pts =
    codeccontext->pts_correction_num_faulty_dts = 0;
    codeccontext->pts_correction_last_pts =
    codeccontext->pts_correction_last_dts = INT64_MIN;

    if (   !CONFIG_GRAY && codeccontext->flags & AV_CODEC_FLAG_GRAY
        && codeccontext->codec_descriptor->type == AVMEDIA_TYPE_VIDEO)
        av_log(codeccontext, AV_LOG_WARNING,
               "gray decoding requested but not enabled at configuration time\n");

    if (   codeccontext->codec->init && (!(codeccontext->active_thread_type&FF_THREAD_FRAME)
        || codeccontext->internal->frame_thread_encoder)) {
        ret = codeccontext->codec->init(codeccontext);
        if (ret < 0) {
            goto free_and_end;
        }
    }

    ret=0;

    if (av_codec_is_decoder(codeccontext->codec)) {
        if (!codeccontext->bit_rate)
            codeccontext->bit_rate = get_bit_rate(codeccontext);
        /* validate channel layout from the decoder */
        if (codeccontext->channel_layout) {
            int channels = av_get_channel_layout_nb_channels(codeccontext->channel_layout);
            if (!codeccontext->channels)
                codeccontext->channels = channels;
            else if (channels != codeccontext->channels) {
                char buf[512];
                av_get_channel_layout_string(buf, sizeof(buf), -1, codeccontext->channel_layout);
                av_log(codeccontext, AV_LOG_WARNING,
                       "Channel layout '%s' with %d channels does not match specified number of channels %d: "
                       "ignoring specified channel layout\n",
                       buf, channels, codeccontext->channels);
                codeccontext->channel_layout = 0;
            }
        }
        if (codeccontext->channels && codeccontext->channels < 0 ||
            codeccontext->channels > FF_SANE_NB_CHANNELS) {
            ret = AVERROR(EINVAL);
            goto free_and_end;
        }
        if (codeccontext->bits_per_coded_sample < 0) {
            ret = AVERROR(EINVAL);
            goto free_and_end;
        }
        if (codeccontext->sub_charenc) {
            if (codeccontext->codec_type != AVMEDIA_TYPE_SUBTITLE) {
                av_log(codeccontext, AV_LOG_ERROR, "Character encoding is only "
                       "supported with subtitles codecs\n");
                ret = AVERROR(EINVAL);
                goto free_and_end;
            } else if (codeccontext->codec_descriptor->props & AV_CODEC_PROP_BITMAP_SUB) {
                av_log(codeccontext, AV_LOG_WARNING, "Codec '%s' is bitmap-based, "
                       "subtitles character encoding will be ignored\n",
                       codeccontext->codec_descriptor->name);
                codeccontext->sub_charenc_mode = FF_SUB_CHARENC_MODE_DO_NOTHING;
            } else {
                /* input character encoding is set for a text based subtitle
                 * codec at this point */
                if (codeccontext->sub_charenc_mode == FF_SUB_CHARENC_MODE_AUTOMATIC)
                    codeccontext->sub_charenc_mode = FF_SUB_CHARENC_MODE_PRE_DECODER;

                if (codeccontext->sub_charenc_mode == FF_SUB_CHARENC_MODE_PRE_DECODER) {
#if CONFIG_ICONV
                    iconv_t cd = iconv_open("UTF-8", codeccontext->sub_charenc);
                    if (cd == (iconv_t)-1) {
                        ret = AVERROR(errno);
                        av_log(codeccontext, AV_LOG_ERROR, "Unable to open iconv context "
                               "with input character encoding \"%s\"\n", codeccontext->sub_charenc);
                        goto free_and_end;
                    }
                    iconv_close(cd);
#else
                    av_log(codeccontext, AV_LOG_ERROR, "Character encoding subtitles "
                           "conversion needs a libavcodec built with iconv support "
                           "for this codec\n");
                    ret = AVERROR(ENOSYS);
                    goto free_and_end;
#endif
                }
            }
        }

#if FF_API_AVCTX_TIMEBASE
        if (codeccontext->framerate.num > 0 && codeccontext->framerate.den > 0)
            codeccontext->time_base = av_inv_q(av_mul_q(codeccontext->framerate, (AVRational){codeccontext->ticks_per_frame, 1}));
#endif
    }
    if (codec->priv_data_size > 0 && codeccontext->priv_data && codec->priv_class) {
        av_assert0(*(const AVClass **)codeccontext->priv_data == codec->priv_class);
    }

end:
    ff_unlock_avcodec(codec);
    if (options) {
        av_dict_free(options);
        *options = tmp;
    }

    return ret;
free_and_end:
    if (codeccontext->codec &&
        (codeccontext->codec->caps_internal & FF_CODEC_CAP_INIT_CLEANUP))
        codeccontext->codec->close(codeccontext);

    if (codec->priv_class && codec->priv_data_size)
        av_opt_free(codeccontext->priv_data);
    av_opt_free(codeccontext);

#if FF_API_CODED_FRAME
FF_DISABLE_DEPRECATION_WARNINGS
    av_frame_free(&codeccontext->coded_frame);
FF_ENABLE_DEPRECATION_WARNINGS
#endif

    av_dict_free(&tmp);
    av_freep(&codeccontext->priv_data);
    if (codeccontext->internal) {
        av_frame_free(&codeccontext->internal->to_free);
        av_frame_free(&codeccontext->internal->compat_decode_frame);
        av_frame_free(&codeccontext->internal->buffer_frame);
        av_packet_free(&codeccontext->internal->buffer_pkt);
        av_packet_free(&codeccontext->internal->last_pkt_props);

        av_packet_free(&codeccontext->internal->ds.in_pkt);
        ff_decode_bsfs_uninit(codeccontext);

        av_freep(&codeccontext->internal->pool);
    }
    av_freep(&codeccontext->internal);
    codeccontext->codec = NULL;
    goto end;
}