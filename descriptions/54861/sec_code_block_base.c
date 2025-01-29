if (s->codec_id == AV_CODEC_ID_SVQ3) {
        *width = FFMAX(*width, 32);
    }

    for (i = 0; i < 4; i++)
        linesize_align[i] = STRIDE_ALIGN;