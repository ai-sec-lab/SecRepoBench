char *p;
    int end_slash = FLB_FALSE;
    struct mk_list *tmp;
    struct mk_list *prev;
    struct mk_list *head;
    struct mk_list *split;
    struct flb_split_entry *entry;
    flb_sds_t out;

    out = flb_sds_create_len(uri, len);
    if (!out) {
        return NULL;
    }

    if (uri[len - 1] == '/') {
        end_slash = FLB_TRUE;
    }