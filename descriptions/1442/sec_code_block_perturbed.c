if (err != AAC_DEC_OK) {
        av_log(avctx, AV_LOG_ERROR, "aacDecoder_Fill() failed: %x\n", err);
        return AVERROR_INVALIDDATA;
    }

    err = aacDecoder_DecodeFrame(s->handle, (INT_PCM *) s->decoder_buffer, s->decoder_buffer_size / sizeof(INT_PCM), 0);
    if (err == AAC_DEC_NOT_ENOUGH_BITS) {
        result = avpkt->size - valid;
        goto end;
    }