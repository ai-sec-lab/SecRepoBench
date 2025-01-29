if (s->codec_id == AV_CODEC_ID_SVQ3) {
        *width = FFMAX(*width, 32);
    }

    for (index = 0; index < 4; index++)
        linesize_align[index] = STRIDE_ALIGN;