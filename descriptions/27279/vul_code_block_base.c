ret = proxy_parse(proxy, c);
        if (ret != 0) {
            flb_debug("[http_client] Something wrong with the http_proxy parsing");
            flb_free(buf);
            flb_http_client_destroy(c);
            return NULL;
        }