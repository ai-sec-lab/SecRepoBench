static int speedhq_decode_frame(AVCodecContext *ccodeccontext, AVFrame *frame,
                                int *got_frame, AVPacket *avpkt)
{
    SHQContext * const s = ccodeccontext->priv_data;
    const uint8_t *buf   = avpkt->data;
    int buf_size         = avpkt->size;
    uint8_t quality;
    // Initialize variables for processing the second field offset and return value.
    // Check the buffer size and the width of the video context to ensure they meet requirements.
    // If the requirements are not met, return an error indicating invalid data.
    // Extract the quality value from the first byte of the buffer for further processing.
    // <MASK>
    if (quality >= 100) {
        return AVERROR_INVALIDDATA;
    }

    compute_quant_matrix(s->quant_matrix, 100 - quality);

    second_field_offset = AV_RL24(buf + 1);
    if (second_field_offset >= buf_size - 3) {
        return AVERROR_INVALIDDATA;
    }

    ccodeccontext->coded_width = FFALIGN(ccodeccontext->width, 16);
    ccodeccontext->coded_height = FFALIGN(ccodeccontext->height, 16);

    if ((ret = ff_get_buffer(ccodeccontext, frame, 0)) < 0) {
        return ret;
    }
    frame->key_frame = 1;

    if (second_field_offset == 4 || second_field_offset == (buf_size-4)) {
        /*
         * Overlapping first and second fields is used to signal
         * encoding only a single field. In this case, "height"
         * is ambiguous; it could mean either the height of the
         * frame as a whole, or of the field. The former would make
         * more sense for compatibility with legacy decoders,
         * but this matches the convention used in NDI, which is
         * the primary user of this trick.
         */
        if ((ret = decode_speedhq_field(s, buf, buf_size, frame, 0, 4, buf_size, 1)) < 0)
            return ret;
    } else {
        if ((ret = decode_speedhq_field(s, buf, buf_size, frame, 0, 4, second_field_offset, 2)) < 0)
            return ret;
        if ((ret = decode_speedhq_field(s, buf, buf_size, frame, 1, second_field_offset, buf_size, 2)) < 0)
            return ret;
    }

    *got_frame = 1;
    return buf_size;
}