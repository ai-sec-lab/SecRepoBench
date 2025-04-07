static int speedhq_decode_frame(AVCodecContext *codec_context, AVFrame *frame,
                                int *got_frame, AVPacket *avpkt)
{
    // Initialize context and packet data variables for processing the frame.
    // Validate basic constraints on packet size and codec context width to ensure data integrity.
    // Extract a quality parameter from the packet data to adjust processing parameters.
    // Calculate a quantization matrix based on the quality parameter to prepare for further decoding.
    // <MASK>

    second_field_offset = AV_RL24(buf + 1);
    if (second_field_offset >= buf_size - 3) {
        return AVERROR_INVALIDDATA;
    }

    codec_context->coded_width = FFALIGN(codec_context->width, 16);
    codec_context->coded_height = FFALIGN(codec_context->height, 16);

    if ((ret = ff_get_buffer(codec_context, frame, 0)) < 0) {
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