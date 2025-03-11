while (ctx->pos < ctx->end) {
                if (*ctx->pos == '\\') {
                    num_escapes++;
                }
                if ((*ctx->pos == quote_char) && (*(ctx->pos-1) != '\\')) {
                    break;
                }
                ctx->pos++;
            }
            if (ctx->pos >= ctx->end) {
                PLIST_OSTEP_ERR("EOF while parsing quoted string at offset %ld\n", ctx->pos - ctx->start);
                ctx->err++;
                goto err_out;
            }
            if (*ctx->pos != quote_char) {
                plist_free_data(data);
                PLIST_OSTEP_ERR("Missing closing quote (%c) at offset %ld\n", quote_char, ctx->pos - ctx->start);
                ctx->err++;
                goto err_out;
            }
            size_t slen = ctx->pos - p;
            ctx->pos++; // skip the closing quote
            char* strbuf = malloc(slen+1);