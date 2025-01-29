frame->nb_samples = s->frame_size * s->frames_per_packet;
    if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
        return ret;

    dst = (float *)frame->extended_data[0];
    for (int i = 0; i < s->frames_per_packet; i++) {
        ret = speex_modes[s->mode].decode(avctx, &s->st[s->mode], &s->gb, dst + i * s->frame_size);
        if (ret < 0)
            return ret;
        if (avctx->channels == 2)
            speex_decode_stereo(dst + i * s->frame_size, s->frame_size, &s->stereo);
    }

    dst = (float *)frame->extended_data[0];
    s->fdsp->vector_fmul_scalar(dst, dst, scale, frame->nb_samples * frame->channels);