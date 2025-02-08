code = pdfi_filter(ctx, stream_obj, compressed_stream, &decompressed_stream, false);
    if (code < 0) {
        pdfi_close_memory_stream(ctx, *Buffer, *new_pdf_stream);
        gs_free_object(ctx->memory, *Buffer, "pdfi_open_memory_stream_from_filtered_stream");
        *Buffer = NULL;
        *new_pdf_stream = NULL;
        return code;
    }
    do {
        byte b[512];
        code = pdfi_read_bytes(ctx, (byte *)&b, 1, 512, decompressed_stream);
        if (code <= 0)
            break;
        decompressed_length+=code;
        if (code < 512)
            break;
    } while (true);
    pdfi_close_file(ctx, decompressed_stream);

    decompressed_Buffer = gs_alloc_bytes(ctx->memory, decompressed_length, "pdfi_open_memory_stream_from_filtered_stream (decompression buffer)");
    if (decompressed_Buffer != NULL) {
        code = srewind(compressed_stream->s);
        if (code >= 0) {
            code = pdfi_filter(ctx, stream_obj, compressed_stream,
                               &decompressed_stream, false);
            if (code >= 0) {
                code = pdfi_read_bytes(ctx, decompressed_Buffer, 1, decompressed_length, decompressed_stream);
                pdfi_close_file(ctx, decompressed_stream);
                code = pdfi_close_memory_stream(ctx, *Buffer, *new_pdf_stream);
                if (code >= 0) {
                    *Buffer = decompressed_Buffer;
                    code = pdfi_open_memory_stream_from_memory(ctx, (unsigned int)decompressed_length,
                                                               *Buffer, new_pdf_stream, retain_ownership);
                } else {
                    *Buffer = NULL;
                    *new_pdf_stream = NULL;
                }
            }
        } else {
            pdfi_close_memory_stream(ctx, *Buffer, *new_pdf_stream);
            gs_free_object(ctx->memory, decompressed_Buffer, "pdfi_open_memory_stream_from_filtered_stream");
            gs_free_object(ctx->memory, Buffer, "pdfi_open_memory_stream_from_filtered_stream");
            *Buffer = NULL;
            *new_pdf_stream = NULL;
            return code;
        }
    } else {
        pdfi_close_memory_stream(ctx, *Buffer, *new_pdf_stream);
        gs_free_object(ctx->memory, Buffer, "pdfi_open_memory_stream_from_filtered_stream");
        *Buffer = NULL;
        *new_pdf_stream = NULL;
        return_error(gs_error_VMerror);
    }
    if (code < 0) {
        gs_free_object(ctx->memory, Buffer, "pdfi_build_function_4");
        *Buffer = NULL;
        *new_pdf_stream = NULL;
        return code;
    }
    return decompressed_length;