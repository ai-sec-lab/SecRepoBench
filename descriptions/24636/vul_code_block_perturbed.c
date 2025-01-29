uint32_t tp_header_size = bytestream2_get_be32u(&s->packed_headers_stream);
                bytestream2_init(&tp->header_tpg, s->packed_headers_stream.buffer, tp_header_size);
                bytestream2_skip(&s->packed_headers_stream, tp_header_size);