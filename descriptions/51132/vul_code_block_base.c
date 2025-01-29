type = flb_ml_type_lookup(tmp);
            if (type == -1) {
                flb_error("[multiline_parser] invalid type '%s'", tmp);
                flb_sds_destroy(tmp);
                goto fconf_error;
            }
            flb_sds_destroy(tmp);