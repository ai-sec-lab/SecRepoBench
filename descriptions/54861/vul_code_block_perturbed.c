if (codeccontext->codec_id == AV_CODEC_ID_H264 || codeccontext->lowres ||
        codeccontext->codec_id == AV_CODEC_ID_VC1  || codeccontext->codec_id == AV_CODEC_ID_WMV3 ||
        codeccontext->codec_id == AV_CODEC_ID_VP5  || codeccontext->codec_id == AV_CODEC_ID_VP6 ||
        codeccontext->codec_id == AV_CODEC_ID_VP6F || codeccontext->codec_id == AV_CODEC_ID_VP6A
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