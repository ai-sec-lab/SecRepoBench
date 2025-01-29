if (ctx->pos >= ctx->end) {
                byte_array_free(bytes);
                plist_free_data(data);
                PLIST_OSTEP_ERR("EOF while parsing data terminator '>' at offset %ld\n", ctx->pos - ctx->start);
                ctx->err++;
                goto err_out;
            }
            if (*ctx->pos != '>') {
                byte_array_free(bytes);
                plist_free_data(data);
                PLIST_OSTEP_ERR("Missing terminating '>' at offset %ld\n", ctx->pos - ctx->start);
                ctx->err++;
                goto err_out;
            }
            ctx->pos++;
            data->buff = bytes->data;
            data->length = bytes->len;
            bytes->data = NULL;
            byte_array_free(bytes);
            *plist = plist_new_node(data);
            parse_skip_ws(ctx);
            break;