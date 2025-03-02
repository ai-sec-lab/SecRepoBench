/* type */
        tmp = get_parser_key(config, cf, s, "type");
        if (!tmp) {
            flb_error("[multiline_parser] no 'type' defined in file '%s'", cfg);
            goto fconf_error;
        }
        else {
            type = flb_ml_type_lookup(tmp);
            if (type == -1) {
                flb_error("[multiline_parser] invalid type '%s'", tmp);
                flb_sds_destroy(tmp);
                goto fconf_error;
            }
            flb_sds_destroy(tmp);
        }

        /* match_string */