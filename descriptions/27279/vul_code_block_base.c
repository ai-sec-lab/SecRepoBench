add_host_and_content_length(c);

    /* Check proxy data */
    if (proxy) {
        flb_debug("[http_client] Using http_proxy: %s", proxy);
        ret = proxy_parse(proxy, c);
        if (ret != 0) {
            flb_debug("[http_client] Something wrong with the http_proxy parsing");
            flb_free(buf);
            flb_http_client_destroy(c);
            return NULL;
        }
    }

    /* 'Read' buffer size */
    c->resp.data = flb_malloc(FLB_HTTP_DATA_SIZE_MAX);