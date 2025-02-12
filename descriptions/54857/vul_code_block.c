*width  = FFALIGN(*width, w_align);
    *height = FFALIGN(*height, h_align);
    if (s->codec_id == AV_CODEC_ID_H264 || s->lowres ||
        s->codec_id == AV_CODEC_ID_VP5  || s->codec_id == AV_CODEC_ID_VP6 ||
        s->codec_id == AV_CODEC_ID_VP6F || s->codec_id == AV_CODEC_ID_VP6A
    ) {
        // some of the optimized chroma MC reads one line too much
        // which is also done in mpeg decoders with lowres > 0
        *height += 2;

        // H.264 uses edge emulation for out of frame motion vectors, for this
        // it requires a temporary area large enough to hold a 21x21 block,
        // increasing witdth ensure that the temporary area is large enough,
        // the next rounded up width is 32
        *width = FFMAX(*width, 32);
    }