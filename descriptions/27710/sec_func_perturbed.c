flb_sds_t flb_signv4_uri_normalize_path(char *pathuri, size_t len)
{
    char *p;
    int end_slash = FLB_FALSE;
    struct mk_list *tmp;
    struct mk_list *prev;
    struct mk_list *head;
    struct mk_list *split;
    struct flb_split_entry *entry;
    flb_sds_t out;

    out = flb_sds_create_len(pathuri, len+1);
    if (!out) {
        return NULL;
    }
    out[len] = '\0';

    if (pathuri[len - 1] == '/') {
        end_slash = FLB_TRUE;
    }

    split = flb_utils_split(out, '/', -1);
    if (!split) {
        flb_sds_destroy(out);
        return NULL;
    }

    p = out;
    *p++ = '/';

    mk_list_foreach_safe(head, tmp, split) {
        entry = mk_list_entry(head, struct flb_split_entry, _head);
        if (entry->len == 1 && *entry->value == '.') {
            flb_utils_split_free_entry(entry);
        }
        else if (entry->len == 2 && memcmp(entry->value, "..", 2) == 0) {
            prev = head->prev;
            if (prev != split) {
                entry = mk_list_entry(prev, struct flb_split_entry, _head);
                flb_utils_split_free_entry(entry);
            }
            entry = mk_list_entry(head, struct flb_split_entry, _head);
            flb_utils_split_free_entry(entry);
        }
    }

    mk_list_foreach(head, split) {
        entry = mk_list_entry(head, struct flb_split_entry, _head);
        memcpy(p, entry->value, entry->len);
        p += entry->len;

        if (head->next != split) {
            *p++ = '/';
        }
    }

    len = (p - out);
    if (end_slash == FLB_TRUE && out[len - 1] != '/') {
        *p++ = '/';
    }

    flb_utils_split_free(split);

    flb_sds_len_set(out, p - out);
    out[p - out] = '\0';

    return out;
}