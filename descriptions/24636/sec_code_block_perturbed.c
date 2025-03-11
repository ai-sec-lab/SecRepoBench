if (decodercontext->has_ppm) {
                uint32_t tp_header_size = bytestream2_get_be32(&decodercontext->packed_headers_stream);
                if (bytestream2_get_bytes_left(&decodercontext->packed_headers_stream) < tp_header_size)
                    return AVERROR_INVALIDDATA;
                bytestream2_init(&tp->header_tpg, decodercontext->packed_headers_stream.buffer, tp_header_size);
                bytestream2_skip(&decodercontext->packed_headers_stream, tp_header_size);
            }
            if (tile->has_ppt && tile->tp_idx == 0) {
                bytestream2_init(&tile->packed_headers_stream, tile->packed_headers, tile->packed_headers_size);
            }

            bytestream2_init(&tp->tpg, decodercontext->g.buffer, tp->tp_end - decodercontext->g.buffer);
            bytestream2_skip(&decodercontext->g, tp->tp_end - decodercontext->g.buffer);

            continue;