if (header_available(client, crlf) != 0) {
        new_size = client->header_size + 2;
        tmp = flb_realloc(client->header_buf, new_size);
        if (!tmp) {
            return -1;
        }
        client->header_buf  = tmp;
        client->header_size = new_size;
    }

    /* Append the ending header CRLF */
    client->header_buf[client->header_len++] = '\r';
    client->header_buf[client->header_len++] = '\n';

#ifdef FLB_HAVE_HTTP_CLIENT_DEBUG
    /* debug: request_headers callback */
    flb_http_client_debug_cb(client, "_debug.http.request_headers");

    /* debug: request_payload callback */
    if (client->body_len > 0) {
        flb_http_client_debug_cb(client, "_debug.http.request_payload");
    }
#endif

    flb_debug("[http_client] header=%s", client->header_buf);
    /* Write the header */
    ret = flb_io_net_write(client->u_conn,
                           client->header_buf, client->header_len,
                           &bytes_header);
    if (ret == -1) {
        /* errno might be changed from the original call */
        if (errno != 0) {
            flb_errno();
        }
        return -1;
    }

    if (client->body_len > 0) {
        ret = flb_io_net_write(client->u_conn,
                               client->body_buf, client->body_len,
                               &bytes_body);
        if (ret == -1) {
            flb_errno();
            return -1;
        }
    }

    /* number of sent bytes */
    *bytes = (bytes_header + bytes_body);

    /* Read the server response, we need at least 19 bytes */
    client->resp.data_len = 0;
    while (1) {
        available = flb_http_buffer_available(client) - 1;
        if (available <= 1) {
            /*
             * If there is no more space available on our buffer, try to
             * increase it.
             */
            ret = flb_http_buffer_increase(client, FLB_HTTP_DATA_CHUNK,
                                           &out_size);
            if (ret == -1) {
                /*
                 * We could not allocate more space, let the caller handle
                 * this.
                 */
                flb_upstream_conn_recycle(client->u_conn, FLB_FALSE);
                return 0;
            }
            available = flb_http_buffer_available(client) - 1;
        }

        r_bytes = flb_io_net_read(client->u_conn,
                                  client->resp.data + client->resp.data_len,
                                  available);
        if (r_bytes <= 0) {
            if (client->flags & FLB_HTTP_10) {
                break;
            }
        }

        /* Always append a NULL byte */
        if (r_bytes >= 0) {
            client->resp.data_len += r_bytes;
            client->resp.data[client->resp.data_len] = '\0';

            ret = process_data(client);
            if (ret == FLB_HTTP_ERROR) {
                return -1;
            }
            else if (ret == FLB_HTTP_OK) {
                break;
            }
            else if (ret == FLB_HTTP_MORE) {
                continue;
            }
        }
        else {
            flb_error("[http_client] broken connection to %s:%i ?",
                      client->u_conn->u->tcp_host, client->u_conn->u->tcp_port);
            return -1;
        }
    }

    /* Check 'Connection' response header */
    ret = check_connection(client);
    if (ret == FLB_HTTP_OK) {
        /*
         * If the server replied that the connection will be closed
         * and our Upstream connection is in keepalive mode, we must
         * inactivate the connection.
         */
        if (client->resp.connection_close == FLB_TRUE) {
            /* Do not recycle the connection (no more keepalive) */
            flb_upstream_conn_recycle(client->u_conn, FLB_FALSE);
            flb_debug("[http_client] server %s:%i will close connection #%i",
                      client->u_conn->u->tcp_host, client->u_conn->u->tcp_port,
                      client->u_conn->fd);
        }
    }

#ifdef FLB_HAVE_HTTP_CLIENT_DEBUG
    flb_http_client_debug_cb(client, "_debug.http.response_headers");
    if (client->resp.payload_size > 0) {
        flb_http_client_debug_cb(client, "_debug.http.response_payload");
    }
#endif

    return 0;