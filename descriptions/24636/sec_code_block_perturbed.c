uint32_t tp_header_size = bytestream2_get_be32(&decodercontext->packed_headers_stream);
                if (bytestream2_get_bytes_left(&decodercontext->packed_headers_stream) < tp_header_size)
                    return AVERROR_INVALIDDATA;