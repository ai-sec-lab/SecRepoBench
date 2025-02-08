int pdfi_set_input_stream(pdf_context *ctx, stream *stm)
{
    byte *Buffer = NULL;
    char *s = NULL;
    float version = 0.0;
    gs_offset_t Offset = 0;
    int64_t bytes = 0, leftover = 0;
    bool found = false;
    int code;

    /* In case of broken PDF files, the repair could run off the end of the
     * file, so make sure that doing so does *not* automagically close the file
     */
    stm->close_at_eod = false;

    ctx->main_stream = (pdf_c_stream *)gs_alloc_bytes(ctx->memory, sizeof(pdf_c_stream), "PDF interpreter allocate main PDF stream");
    if (ctx->main_stream == NULL)
        return_error(gs_error_VMerror);
    memset(ctx->main_stream, 0x00, sizeof(pdf_c_stream));
    ctx->main_stream->s = stm;

    Buffer = gs_alloc_bytes(ctx->memory, BUF_SIZE, "PDF interpreter - allocate working buffer for file validation");
    if (Buffer == NULL) {
        code = gs_error_VMerror;
        goto error;
    }

    /* Determine file size */
    pdfi_seek(ctx, ctx->main_stream, 0, SEEK_END);
    ctx->main_stream_length = pdfi_tell(ctx->main_stream);
    Offset = BUF_SIZE;
    bytes = BUF_SIZE;
    pdfi_seek(ctx, ctx->main_stream, 0, SEEK_SET);

    bytes = Offset = min(BUF_SIZE - 1, ctx->main_stream_length);

    if (ctx->args.pdfdebug)
        dmprintf(ctx->memory, "%% Reading header\n");

    bytes = pdfi_read_bytes(ctx, Buffer, 1, Offset, ctx->main_stream);
    if (bytes <= 0) {
        emprintf(ctx->memory, "Failed to read any bytes from input stream\n");
        code = gs_error_ioerror;
        goto error;
    }
    if (bytes < 8) {
        emprintf(ctx->memory, "Failed to read enough bytes for a valid PDF header from input stream\n");
        code = gs_error_ioerror;
        goto error;
    }
    Buffer[Offset] = 0x00;

    /* First check for existence of header */
    s = strstr((char *)Buffer, "%PDF");
    if (s == NULL) {
        char extra_info[gp_file_name_sizeof];

        if (ctx->filename)
            gs_sprintf(extra_info, "%% File %s does not appear to be a PDF file (no %%PDF in first 2Kb of file)\n", ctx->filename);
        else
            gs_sprintf(extra_info, "%% File does not appear to be a PDF stream (no %%PDF in first 2Kb of stream)\n");

        pdfi_set_error(ctx, 0, NULL, E_PDF_NOHEADER, "pdfi_set_input_stream", extra_info);
    } else {
        /* Now extract header version (may be overridden later) */
        if (sscanf(s + 5, "%f", &version) != 1) {
            ctx->HeaderVersion = 0;
            pdfi_set_error(ctx, 0, NULL, E_PDF_NOHEADERVERSION, "pdfi_set_input_stream", (char *)"%% Unable to read PDF version from header\n");
        }
        else {
            ctx->HeaderVersion = version;
        }
        if (ctx->args.pdfdebug)
            dmprintf1(ctx->memory, "%% Found header, PDF version is %f\n", ctx->HeaderVersion);
    }

    /* Jump to EOF and scan backwards looking for startxref */
    pdfi_seek(ctx, ctx->main_stream, 0, SEEK_END);

    if (ctx->args.pdfdebug)
        dmprintf(ctx->memory, "%% Searching for 'startxerf' keyword\n");

    /* Initially read min(BUF_SIZE, file_length) bytes of data to the buffer */
    bytes = Offset;

    do {
        byte *last_lineend = NULL;
        uint32_t read;

        if (pdfi_seek(ctx, ctx->main_stream, ctx->main_stream_length - Offset, SEEK_SET) != 0) {
            emprintf1(ctx->memory, "File is smaller than %"PRIi64" bytes\n", (int64_t)Offset);
            code = gs_error_ioerror;
            goto error;
        }
        read = pdfi_read_bytes(ctx, Buffer, 1, bytes, ctx->main_stream);

        if (read <= 0) {
            emprintf1(ctx->memory, "Failed to read %"PRIi64" bytes from file\n", (int64_t)bytes);
            code = gs_error_ioerror;
            goto error;
        }

        /* When reading backwards, if we ran out of data in the last buffer while looking
         * for a 'startxref, but we had found a linefeed, then we preserved everything
         * from the beginning of the buffer up to that linefeed, by copying it to the end
         * of the buffer and reducing the number of bytes to read so that it should have filled
         * in the gap. If we didn't read enough bytes, then we have a gap between the end of
         * the data we just read and the leftover data from teh last buffer. Move the preserved
         * data down to meet the end of the data we just read.
         */
        if (bytes != read && leftover != 0)
            memcpy(Buffer + read, Buffer + bytes, leftover);

        /* As above, if we had any leftover data from the last buffer then increase the
         * number of bytes available by that amount. We increase 'bytes' (the number of bytes
         * to read) to the same value, which should mean we read an entire buffer's worth. Of
         * course if we have any data left out of this buffer we'll reduce bytes again...
         */
        read = bytes = read + leftover;

        /* Now search backwards in the buffer for the startxref token */
        while(read) {
            if (memcmp(Buffer + read - 9, "startxref", 9) == 0) {
                found = true;
                break;
            } else {
                if (Buffer[read - 1] == 0x0a || Buffer[read - 1] == 0x0d)
                    last_lineend = Buffer + read;
            }
            read--;
        }
        if (found) {
            byte *b = Buffer + read;

            /* Success! stop now */
            if(sscanf((char *)b, " %"PRIdOFFSET"", &ctx->startxref) != 1) {
                dmprintf(ctx->memory, "Unable to read offset of xref from PDF file\n");
            }
            break;
        } else {
            // <MASK>
        }

        Offset += bytes;
    } while(Offset < ctx->main_stream_length);

    if (!found)
        pdfi_set_error(ctx, 0, NULL, E_PDF_NOSTARTXREF, "pdfi_set_input_stream", NULL);

    code = pdfi_init_file(ctx);

error:
    gs_free_object(ctx->memory, Buffer, "PDF interpreter - allocate working buffer for file validation");
    return code;
}