struct flb_parser *flb_parser_create(const char *name, const char *format,
                                     const char *p_regex,
                                     const char *time_fmt, const char *time_key,
                                     const char *time_offset,
                                     int time_keep,
                                     struct flb_parser_types *types,
                                     int types_len,
                                     struct mk_list *decoders,
                                     struct flb_config *config)
{
    int status;
    int len;
    int diff = 0;
    int size;
    int is_epoch = FLB_FALSE;
    char *tmp;
    char *timeptr;
    struct mk_list *head;
    struct flb_parser *p;
    struct flb_regex *regex;

    /* Iterate current parsers and make sure the new one don't exists */
    mk_list_foreach(head, &config->parsers) {
        p = mk_list_entry(head, struct flb_parser, _head);
        if (strcmp(p->name, name) == 0) {
            flb_error("[parser] parser named '%s' already exists, skip.",
                      name);
            return NULL;
        }
    }

    /* Allocate context */
    p = flb_calloc(1, sizeof(struct flb_parser));
    if (!p) {
        flb_errno();
        return NULL;
    }
    p->decoders = decoders;
    mk_list_add(&p->_head, &config->parsers);

    /* Format lookup */
    if (strcasecmp(format, "regex") == 0) {
        p->type = FLB_PARSER_REGEX;
    }
    else if (strcasecmp(format, "json") == 0) {
        p->type = FLB_PARSER_JSON;
    }
    else if (strcmp(format, "ltsv") == 0) {
        p->type = FLB_PARSER_LTSV;
    }
    else if (strcmp(format, "logfmt") == 0) {
        p->type = FLB_PARSER_LOGFMT;
    }
    else {
        flb_error("[parser:%s] Invalid format %s", name, format);
        flb_free(p);
        return NULL;
    }

    if (p->type == FLB_PARSER_REGEX) {
        if (!p_regex) {
            flb_error("[parser:%s] Invalid regex pattern", name);
            flb_free(p);
            return NULL;
        }

        regex = flb_regex_create(p_regex);
        if (!regex) {
            flb_error("[parser:%s] Invalid regex pattern %s", name, p_regex);
            flb_free(p);
            return NULL;
        }
        p->regex = regex;
        p->p_regex = flb_strdup(p_regex);
    }

    p->name = flb_strdup(name);

    if (time_fmt) {
        p->time_fmt_full = flb_strdup(time_fmt);
        p->time_fmt = flb_strdup(time_fmt);

        /* Check if the format is considering the year */
        // Check if the time format string contains a year specifier ("%Y" or "%y").
        // If found, set a flag indicating the presence of a year in the format.
        // If the format contains "%s", set it as an epoch format and indicate it includes a year.
        // If neither is found, allocate memory to prepend a year ("%Y") to the time format string.
        // Check if the time format contains timezone indicators ("%z", "%Z", etc.).
        // If timezone support is enabled, set a flag indicating timezone is included.
        // Otherwise, log an error and clean up if the platform does not support timezone parsing.
        // Determine the correct format string to use for fractional seconds handling.
        // If fractional seconds ("%L") are found, truncate the format string at that position
        // and store a pointer to the location where fractional seconds should start.
        // Process an optional fixed timezone offset if provided, adjusting the offset variable.
        // Return NULL on failure and perform necessary cleanup in case of errors.
        // <MASK>
    }

    if (time_key) {
        p->time_key = flb_strdup(time_key);
    }

    p->time_keep = time_keep;
    p->types = types;
    p->types_len = types_len;
    return p;
}