static int decode_frame(AVCodecContext *avctx, void *data, int *got_frame,
                        AVPacket *avpkt)
{
    const uint8_t *buf = avpkt->data;
    int buf_size       = avpkt->size;
    HYuvContext *s = avctx->priv_data;
    const int width  = s->width;
    const int height = s->height;
    ThreadFrame frame = { .f = data };
    AVFrame *const p = data;
    int slice, table_size = 0, ret, nb_slices;
    unsigned slices_info_offset;
    int slice_height;

    if (buf_size < (width * height + 7)/8)
        return AVERROR_INVALIDDATA;

    av_fast_padded_malloc(&s->bitstream_buffer,
                   &s->bitstream_buffer_size,
                   buf_size);
    if (!s->bitstream_buffer)
        return AVERROR(ENOMEM);

    s->bdsp.bswap_buf((uint32_t *) s->bitstream_buffer,
                      (const uint32_t *) buf, buf_size / 4);

    if ((ret = ff_thread_get_buffer(avctx, &frame, 0)) < 0)
        return ret;

    if (s->context) {
        table_size = read_huffman_tables(s, s->bitstream_buffer, buf_size);
        if (table_size < 0)
            return table_size;
    }

    if ((unsigned) (buf_size - table_size) >= INT_MAX / 8)
        return AVERROR_INVALIDDATA;

    s->last_slice_end = 0;

    if (avctx->codec_id == AV_CODEC_ID_HYMT &&
        (buf_size > 32 && AV_RL32(avpkt->data + buf_size - 16) == 0)) {
        // Check if the codec ID is AV_CODEC_ID_HYMT and validate the buffer size
        // and specific conditions at the end of the packet data. This helps determine
        // the number of slices, slice height, and offset for slice information.
        // The conditions ensure that the calculated offsets and sizes do not exceed
        // the available buffer, and that the slice height is positive. If the conditions
        // are not met, an error is returned. If the codec ID does not match, default to
        // a single slice covering the full height.
        // <MASK>
    } else {
        slice_height = height;
        nb_slices = 1;
    }

    for (slice = 0; slice < nb_slices; slice++) {
        int y_offset, slice_offset, slice_size;

        if (nb_slices > 1) {
            slice_offset = AV_RL32(avpkt->data + slices_info_offset + slice * 8);
            slice_size = AV_RL32(avpkt->data + slices_info_offset + slice * 8 + 4);
            y_offset = height - (slice + 1) * slice_height;
            s->bdsp.bswap_buf((uint32_t *)s->bitstream_buffer,
                              (const uint32_t *)(buf + slice_offset), slice_size / 4);
        } else {
            y_offset = 0;
            slice_offset = 0;
            slice_size = buf_size;
        }

        ret = decode_slice(avctx, p, slice_height, slice_size, y_offset, table_size);
        emms_c();
        if (ret < 0)
            return ret;
    }

    *got_frame = 1;

    return (get_bits_count(&s->gb) + 31) / 32 * 4 + table_size;
}