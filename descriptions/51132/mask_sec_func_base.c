static int multiline_parser_conf_file(const char *cfg, struct flb_cf *cf,
                                      struct flb_config *config)
{
    int ret;
    int type;
    flb_sds_t name;
    flb_sds_t match_string;
    int negate;
    flb_sds_t key_content;
    flb_sds_t key_pattern;
    flb_sds_t key_group;
    flb_sds_t parser;
    flb_sds_t tmp;
    int flush_timeout;
    struct flb_parser *parser_ctx = NULL;
    struct mk_list *head;
    struct flb_cf_section *s;
    struct flb_ml_parser *ml_parser;

    /* read all 'multiline_parser' sections */
    mk_list_foreach(head, &cf->multiline_parsers) {
        ml_parser = NULL;
        name = NULL;
        type = -1;
        match_string = NULL;
        negate = FLB_FALSE;
        key_content = NULL;
        key_pattern = NULL;
        key_group = NULL;
        parser = NULL;
        flush_timeout = -1;
        tmp = NULL;

        s = mk_list_entry(head, struct flb_cf_section, _head_section);

        /* name */
        name = get_parser_key(config, cf, s, "name");
        if (!name) {
            flb_error("[multiline_parser] no 'name' defined in file '%s'", cfg);
            goto fconf_error;
        }

        // <MASK>
        match_string = get_parser_key(config, cf, s, "match_string");

        /* negate */
        tmp = get_parser_key(config, cf, s, "negate");
        if (tmp) {
            negate = flb_utils_bool(tmp);
            flb_sds_destroy(tmp);
        }

        /* key_content */
        key_content = get_parser_key(config, cf, s, "key_content");

        /* key_pattern */
        key_pattern = get_parser_key(config, cf, s, "key_pattern");

        /* key_group */
        key_group = get_parser_key(config, cf, s, "key_group");

        /* parser */
        parser = get_parser_key(config, cf, s, "parser");

        /* flush_timeout */
        tmp = get_parser_key(config, cf, s, "flush_timeout");
        if (tmp) {
            flush_timeout = atoi(tmp);
        }

        if (parser) {
            parser_ctx = flb_parser_get(parser, config);
        }
        ml_parser = flb_ml_parser_create(config, name, type, match_string,
                                         negate, flush_timeout, key_content,
                                         key_group, key_pattern,
                                         parser_ctx, parser);
        if (!ml_parser) {
            goto fconf_error;
        }

        /* if type is regex, process rules */
        if (type == FLB_ML_REGEX) {
            ret = multiline_load_regex_rules(ml_parser, s, config);
            if (ret != 0) {
                goto fconf_error;
            }
        }

        flb_sds_destroy(name);
        flb_sds_destroy(match_string);
        flb_sds_destroy(key_content);
        flb_sds_destroy(key_pattern);
        flb_sds_destroy(key_group);
        flb_sds_destroy(parser);
        flb_sds_destroy(tmp);
    }

    return 0;

 fconf_error:
    if (ml_parser) {
        flb_ml_parser_destroy(ml_parser);
    }
    flb_sds_destroy(name);
    flb_sds_destroy(match_string);
    flb_sds_destroy(key_content);
    flb_sds_destroy(key_pattern);
    flb_sds_destroy(key_group);
    flb_sds_destroy(parser);
    flb_sds_destroy(tmp);

    return -1;
}