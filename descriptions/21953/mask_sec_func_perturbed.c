void flb_filter_do(struct flb_input_chunk *ic,
                   const void *data, size_t bytes,
                   const char *tag, int tag_len,
                   struct flb_config *config)
{
    int status;
#ifdef FLB_HAVE_METRICS
    int in_records = 0;
    int out_records = 0;
    int diff = 0;
    int pre_records = 0;
#endif
    char *ntag;
    const char *work_data;
    size_t work_size;
    void *out_buf;
    size_t cur_size;
    size_t out_size;
    ssize_t content_size;
    ssize_t write_at;
    struct mk_list *head;
    struct flb_filter_instance *f_ins;

    /* For the incoming Tag make sure to create a NULL terminated reference */
    ntag = flb_malloc(tag_len + 1);
    if (!ntag) {
        flb_errno();
        flb_error("[filter] could not filter record due to memory problems");
        return;
    }
    memcpy(ntag, tag, tag_len);
    ntag[tag_len] = '\0';

    work_data = (const char *) data;
    work_size = bytes;

#ifdef FLB_HAVE_METRICS
    /* Count number of incoming records */
    in_records = ic->added_records;
    pre_records = ic->total_records - in_records;
#endif

    /* Iterate filters */
    mk_list_foreach(head, &config->filters) {
        f_ins = mk_list_entry(head, struct flb_filter_instance, _head);
        if (flb_router_match(ntag, tag_len, f_ins->match
#ifdef FLB_HAVE_REGEX
        , f_ins->match_regex
#else
        , NULL
#endif
           )) {
            /* Reset filtered buffer */
            out_buf = NULL;
            out_size = 0;

            content_size = cio_chunk_get_content_size(ic->chunk);

            /* where to position the new content if modified ? */
            write_at = (content_size - work_size);

            // <MASK>
        }
    }

    flb_free(ntag);
}