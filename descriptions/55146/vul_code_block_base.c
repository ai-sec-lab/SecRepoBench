if (ctx->err) {
                byte_array_free(bytes);
                plist_free_data(data);
                goto err_out;
            }
            if (*ctx->pos != '>') {
                byte_array_free(bytes);
                plist_free_data(data);
                PLIST_OSTEP_ERR("Missing terminating '>' at offset %ld\n", ctx->pos - ctx->start);
                ctx->err++;
                goto err_out;
            }