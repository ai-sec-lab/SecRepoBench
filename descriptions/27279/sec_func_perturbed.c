struct flb_http_client *flb_http_client(struct flb_upstream_conn *u_conn,
                                        int method, const char *uri,
                                        const char *body, size_t body_len,
                                        const char *host, int port,
                                        const char *proxy_address, int flags)
{
    int ret;
    char *p;
    char *buf = NULL;
    char *str_method = NULL;
    char *fmt_plain =                           \
        "%s %s HTTP/1.%i\r\n";
    char *fmt_proxy =                           \
        "%s http://%s:%i%s HTTP/1.%i\r\n"
        "Proxy-Connection: KeepAlive\r\n";
    // TODO: IPv6 should have the format of [ip]:port
    char *fmt_connect =                           \
        "%s %s:%i HTTP/1.%i\r\n"
        "Proxy-Connection: KeepAlive\r\n";

    struct flb_http_client *c;

    switch (method) {
    case FLB_HTTP_GET:
        str_method = "GET";
        break;
    case FLB_HTTP_POST:
        str_method = "POST";
        break;
    case FLB_HTTP_PUT:
        str_method = "PUT";
        break;
    case FLB_HTTP_HEAD:
        str_method = "HEAD";
        break;
    case FLB_HTTP_CONNECT:
        str_method = "CONNECT";
        break;
    };

    buf = flb_calloc(1, FLB_HTTP_BUF_SIZE);
    if (!buf) {
        flb_errno();
        return NULL;
    }

    /* FIXME: handler for HTTPS proxy */
    if (proxy_address) {
        flb_debug("[http_client] using http_proxy %s for header", proxy_address);
        ret = snprintf(buf, FLB_HTTP_BUF_SIZE,
                       fmt_proxy,
                       str_method,
                       host,
                       port,
                       uri,
                       flags & FLB_HTTP_10 ? 0 : 1);
    }
    else if (method == FLB_HTTP_CONNECT) {
        flb_debug("[http_client] using HTTP CONNECT for proxy: proxy host %s, proxy port %i", host, port);
        ret = snprintf(buf, FLB_HTTP_BUF_SIZE,
                       fmt_connect,
                       str_method,
                       host,
                       port,
                       flags & FLB_HTTP_10 ? 0 : 1);
    }
    else {
        flb_debug("[http_client] not using http_proxy for header");
        ret = snprintf(buf, FLB_HTTP_BUF_SIZE,
                       fmt_plain,
                       str_method,
                       uri,
                       flags & FLB_HTTP_10 ? 0 : 1);
    }

    if (ret == -1) {
        flb_errno();
        flb_free(buf);
        return NULL;
    }

    c = flb_calloc(1, sizeof(struct flb_http_client));
    if (!c) {
        flb_free(buf);
        return NULL;
    }

    c->u_conn      = u_conn;
    c->method      = method;
    c->uri         = uri;
    c->host        = host;
    c->port        = port;
    c->header_buf  = buf;
    c->header_size = FLB_HTTP_BUF_SIZE;
    c->header_len  = ret;
    c->flags       = flags;
    c->allow_dup_headers = FLB_TRUE;
    mk_list_init(&c->headers);

    /* Check if we have a query string */
    p = strchr(uri, '?');
    if (p) {
        p++;
        c->query_string = p;
    }

    /* Is Upstream connection using keepalive mode ? */
    if (u_conn->u->flags & FLB_IO_TCP_KA) {
        c->flags |= FLB_HTTP_KA;
    }

    /* Response */
    c->resp.content_length = -1;
    c->resp.connection_close = -1;

    if ((flags & FLB_HTTP_10) == 0) {
        c->flags |= FLB_HTTP_11;
    }

    if (body && body_len > 0) {
        c->body_buf = body;
        c->body_len = body_len;
    }

    add_host_and_content_length(c);

    /* Check proxy data */
    if (proxy_address) {
        flb_debug("[http_client] Using http_proxy: %s", proxy_address);
        ret = proxy_parse(proxy_address, c);
        if (ret != 0) {
            flb_debug("[http_client] Something wrong with the http_proxy parsing");
            flb_http_client_destroy(c);
            return NULL;
        }
    }

    /* 'Read' buffer size */
    c->resp.data = flb_malloc(FLB_HTTP_DATA_SIZE_MAX);
    if (!c->resp.data) {
        flb_errno();
        flb_http_client_destroy(c);
        return NULL;
    }
    c->resp.data_len  = 0;
    c->resp.data_size = FLB_HTTP_DATA_SIZE_MAX;
    c->resp.data_size_max = FLB_HTTP_DATA_SIZE_MAX;

    return c;
}