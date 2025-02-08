int pdfi_read_token(pdf_context *ctx, pdf_c_stream *s, uint32_t indirect_num, uint32_t indirect_gen)
{
    int32_t bytes = 0;
    char Buffer[256];
    int code;

    pdfi_skip_white(ctx, s);

    bytes = pdfi_read_bytes(ctx, (byte *)Buffer, 1, 1, s);
    if (bytes < 0)
        return (gs_error_ioerror);
    if (bytes == 0 && s->eof)
        return 0;

    switch(Buffer[0]) {
        case 0x30:
        case 0x31:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x35:
        case 0x36:
        case 0x37:
        case 0x38:
        case 0x39:
        case '+':
        case '-':
        case '.':
            pdfi_unread(ctx, s, (byte *)&Buffer[0], 1);
            code = pdfi_read_num(ctx, s, indirect_num, indirect_gen);
            // <MASK>
        case '/':
            code = pdfi_read_name(ctx, s, indirect_num, indirect_gen);
            if (code < 0)
                return code;
            return 1;
            break;
        case '<':
            bytes = pdfi_read_bytes(ctx, (byte *)&Buffer[1], 1, 1, s);
            if (bytes <= 0)
                return (gs_error_ioerror);
            if (iswhite(Buffer[1])) {
                code = pdfi_skip_white(ctx, s);
                if (code < 0)
                    return code;
                bytes = pdfi_read_bytes(ctx, (byte *)&Buffer[1], 1, 1, s);
            }
            if (Buffer[1] == '<') {
                if (ctx->args.pdfdebug)
                    dmprintf (ctx->memory, " <<\n");
                code = pdfi_mark_stack(ctx, PDF_DICT_MARK);
                if (code < 0)
                    return code;
                return 1;
            } else {
                if (Buffer[1] == '>') {
                    pdfi_unread(ctx, s, (byte *)&Buffer[1], 1);
                    code =  pdfi_read_hexstring(ctx, s, indirect_num, indirect_gen);
                    if (code < 0)
                        return code;
                    return 1;
                } else {
                    if (ishex(Buffer[1])) {
                        pdfi_unread(ctx, s, (byte *)&Buffer[1], 1);
                        code = pdfi_read_hexstring(ctx, s, indirect_num, indirect_gen);
                        if (code < 0)
                            return code;
                    }
                    else
                        return_error(gs_error_syntaxerror);
                }
            }
            break;
        case '>':
            bytes = pdfi_read_bytes(ctx, (byte *)&Buffer[1], 1, 1, s);
            if (bytes <= 0)
                return (gs_error_ioerror);
            if (Buffer[1] == '>') {
                code = pdfi_dict_from_stack(ctx, indirect_num, indirect_gen, false);
                if (code < 0)
                    return code;
                return 1;
            }
            else {
                pdfi_unread(ctx, s, (byte *)&Buffer[1], 1);
                return_error(gs_error_syntaxerror);
            }
            break;
        case '(':
            code = pdfi_read_string(ctx, s, indirect_num, indirect_gen);
            if (code < 0)
                return code;
            return 1;
            break;
        case '[':
            if (ctx->args.pdfdebug)
                dmprintf (ctx->memory, "[");
            code = pdfi_mark_stack(ctx, PDF_ARRAY_MARK);
            if (code < 0)
                return code;
            return 1;
            break;
        case ']':
            code = pdfi_array_from_stack(ctx, indirect_num, indirect_gen);
            // <MASK>
        case '{':
            if (ctx->args.pdfdebug)
                dmprintf (ctx->memory, "{");
            code = pdfi_mark_stack(ctx, PDF_PROC_MARK);
            if (code < 0)
                return code;
            return 1;
            break;
        case '}':
            pdfi_clear_to_mark(ctx);
            return pdfi_read_token(ctx, s, indirect_num, indirect_gen);
            break;
        case '%':
            pdfi_skip_comment(ctx, s);
            return pdfi_read_token(ctx, s, indirect_num, indirect_gen);
            break;
        default:
            if (isdelimiter(Buffer[0])) {
                if (ctx->args.pdfstoponerror)
                    return_error(gs_error_syntaxerror);
                return pdfi_read_token(ctx, s, indirect_num, indirect_gen);
            }
            pdfi_unread(ctx, s, (byte *)&Buffer[0], 1);
            code = pdfi_read_keyword(ctx, s, indirect_num, indirect_gen);
            if (code < 0)
                return code;
            return 1;
            break;
    }
    return 1;
}