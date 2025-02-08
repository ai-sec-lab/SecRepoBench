static int read_xref_section(pdf_context *ctx, pdf_c_stream *s, uint64_t *xrefstart, uint64_t *section_size)
{
    int code = 0, i, j;
    int start = 0;
    int size = 0;
    int64_t bytes = 0;
    char Buffer[21];

    *xrefstart = *section_size = 0;

    if (ctx->args.pdfdebug)
        dmprintf(ctx->memory, "\n%% Reading xref section\n");

    code = pdfi_read_bare_int(ctx, ctx->main_stream, &start);
    if (code < 0) {
        /* Not an int, might be a keyword */
        code = pdfi_read_bare_keyword(ctx, ctx->main_stream);
        if (code < 0)
            return code;

        if (code != TOKEN_TRAILER) {
            /* element is not an integer, and not a keyword - not a valid xref */
            return_error(gs_error_typecheck);
        }
        return 1;
    }

    // <MASK>
}