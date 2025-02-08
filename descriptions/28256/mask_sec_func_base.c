int flb_http_do(struct flb_http_client *c, size_t *bytes)
{
    int ret;
    int r_bytes;
    int crlf = 2;
    int new_size;
    ssize_t available;
    size_t out_size;
    size_t bytes_header = 0;
    size_t bytes_body = 0;
    char *tmp;

    /* Append pending headers */
    ret = http_headers_compose(c);
    if (ret == -1) {
        return -1;
    }

    /* check enough space for the ending CRLF */
    // <MASK>
}