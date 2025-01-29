marker = pkt->data[pkt->size - 1];
    if ((marker & 0xe0) == 0xc0) {
        int nbytes = 1 + ((marker >> 3) & 0x3);
        int n_frames = 1 + (marker & 0x7), idx_sz = 2 + n_frames * nbytes;

        uses_superframe_syntax = pkt->size >= idx_sz && pkt->data[pkt->size - idx_sz] == marker;
    }

    if ((res = init_get_bits8(&gb, pkt->data, pkt->size)) < 0)
        goto done;

    get_bits(&gb, 2); // frame marker
    profile  = get_bits1(&gb);
    profile |= get_bits1(&gb) << 1;
    if (profile == 3) profile += get_bits1(&gb);

    if (get_bits1(&gb)) {
        isinvisible = 0;
    } else {
        get_bits1(&gb); // keyframe
        isinvisible = !get_bits1(&gb);
    }

    if (uses_superframe_syntax && s->n_cache > 0) {
        av_log(ctx, AV_LOG_ERROR,
               "Mixing of superframe syntax and naked VP9 frames not supported\n");
        res = AVERROR(ENOSYS);
        goto done;
    } else if ((!isinvisible || uses_superframe_syntax) && !s->n_cache) {
        // passthrough
        return 0;
    } else if (s->n_cache + 1 >= MAX_CACHE) {
        av_log(ctx, AV_LOG_ERROR,
               "Too many invisible frames\n");
        res = AVERROR_INVALIDDATA;
        goto done;
    }

    av_packet_move_ref(s->cache[s->n_cache++], pkt);

    if (isinvisible) {
        return AVERROR(EAGAIN);
    }
    av_assert0(s->n_cache > 0);

    // build superframe
    if ((res = merge_superframe(s->cache, s->n_cache, pkt)) < 0)
        goto done;

    res = av_packet_copy_props(pkt, s->cache[s->n_cache - 1]);
    if (res < 0)
        goto done;

    for (n = 0; n < s->n_cache; n++)
        av_packet_unref(s->cache[n]);
    s->n_cache = 0;

done:
    if (res < 0)
        av_packet_unref(pkt);
    return res;