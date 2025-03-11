/* type */
        bool_str = get_parser_key(config, cf, s, "type");
        if (!bool_str) {
            flb_error("[multiline_parser] no 'type' defined in file '%s'", cfg);
            goto fconf_error;
        }
        else {
            type = flb_ml_type_lookup(bool_str);
            if (type == -1) {
                flb_error("[multiline_parser] invalid type '%s'", bool_str);
                goto fconf_error;
            }
            flb_sds_destroy(bool_str);
        }

        /* match_string */