static int process_chunked_data(struct flb_http_client *c)
{
    long len;
    long drop;
    long val;
    char *p;
    char tmp[32];
    struct flb_http_response *r = &c->resp;

 chunk_start:
    p = strstr(r->chunk_processed_end, "\r\n");
    if (!p) {
        return FLB_HTTP_MORE;
    }

    /* Hexa string length */
    len = (p - r->chunk_processed_end);
    if ((len > sizeof(tmp) - 1) || len == 0) {
        return FLB_HTTP_ERROR;
    }
    p += 2;

    /* Copy hexa string to temporary buffer */
    memcpy(tmp, r->chunk_processed_end, len);
    tmp[len] = '\0';

    /* Convert hexa string to decimal */
    errno = 0;
    val = strtol(tmp, NULL, 16);
    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
        || (errno != 0 && val == 0)) {
        flb_errno();
        return FLB_HTTP_ERROR;
    }
    // <MASK>
}