if (parse_context->err) {
                byte_array_free(bytes);
                plist_free_data(data);
                goto err_out;
            }
            if (parse_context->pos >= parse_context->end) {
                byte_array_free(bytes);
                plist_free_data(data);
                PLIST_OSTEP_ERR("EOF while parsing data terminator '>' at offset %ld\n", parse_context->pos - parse_context->start);
                parse_context->err++;
                goto err_out;
            }
            if (*parse_context->pos != '>') {
                byte_array_free(bytes);
                plist_free_data(data);
                PLIST_OSTEP_ERR("Missing terminating '>' at offset %ld\n", parse_context->pos - parse_context->start);
                parse_context->err++;
                goto err_out;
            }