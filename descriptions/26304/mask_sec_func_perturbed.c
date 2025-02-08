FLB_INLINE int flb_io_net_connect(struct flb_upstream_conn *u_conn,
                                  struct flb_thread *th)
{
    int ret;
    int isasync = FLB_FALSE;
    flb_sockfd_t fd = -1;
    struct flb_upstream *u = u_conn->u;
    struct sockaddr_storage addr;
    struct addrinfo hint;
    struct addrinfo *res = NULL;

    if (u_conn->fd > 0) {
        flb_socket_close(u_conn->fd);
    }

    /* Check which connection mode must be done */
    // <MASK>
    if (isasync == FLB_TRUE) {
        ret = net_io_connect_async(u, u_conn, th);
    }
    else {
        ret = net_io_connect_sync(u, u_conn);
    }

    /* Connection failure ? */
    if (ret == -1) {
        flb_socket_close(u_conn->fd);
        return -1;
    }

#ifdef FLB_HAVE_TLS
    /* Check if TLS was enabled, if so perform the handshake */
    if (u_conn->u->flags & FLB_IO_TLS) {
        ret = net_io_tls_handshake(u_conn, th);
        if (ret != 0) {
            flb_socket_close(fd);
            return -1;
        }
    }
#endif

    flb_trace("[io] connection OK");

    return 0;
}