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