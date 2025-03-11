{
            /* attempt to parse this as 'strings' data */
            plist_free(*plist);
            *plist = NULL;
            plist_t pl = plist_new_dict();
            ctx.pos = plistopenstep;
            parse_dict_data(&ctx, pl);
            if (ctx.err > 0) {
                plist_free(pl);
                PLIST_OSTEP_ERR("Failed to parse strings data\n");
                err = PLIST_ERR_PARSE;
            } else {
                *plist = pl;
            }
        }