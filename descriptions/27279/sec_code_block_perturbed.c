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