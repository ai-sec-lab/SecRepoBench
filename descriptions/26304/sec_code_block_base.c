if (th) {
        async = flb_upstream_is_async(u);
    }
    else {
        async = FLB_FALSE;
    }

    /*
     * If the net.source_address was set, we need to determinate the address
     * type (for socket type creation) and bind it.
     *
     * Note that this routine overrides the behavior of the 'ipv6' configuration
     * property.
     */
    if (u->net.source_address) {
        memset(&hint, '\0', sizeof hint);

        hint.ai_family = PF_UNSPEC;
        hint.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV | AI_PASSIVE;

        ret = getaddrinfo(u->net.source_address, NULL, &hint, &res);
        if (ret == -1) {
            flb_errno();
            flb_error("[io] cannot parse source_address=%s",
                      u->net.source_address);
            return -1;
        }

        if (res->ai_family == AF_INET) {
            fd = flb_net_socket_create(AF_INET, async);
        }
        else if (res->ai_family == AF_INET6) {
            fd = flb_net_socket_create(AF_INET6, async);
        }
        else {
            flb_error("[io] could not create socket for "
                      "source_address=%s, unknown ai_family",
                      u->net.source_address);
            freeaddrinfo(res);
            return -1;
        }

        if (fd == -1) {
            flb_error("[io] could not create an %s socket for "
                      "source_address=%s",
                      res->ai_family == AF_INET ? "IPv4": "IPv6",
                      u->net.source_address);
            freeaddrinfo(res);
            return -1;
        }

        /* Bind the address */
        memcpy(&addr, res->ai_addr, res->ai_addrlen);
        freeaddrinfo(res);
        ret = bind(fd, (struct sockaddr *) &addr, sizeof(addr));
        if (ret == -1) {
            flb_errno();
            flb_socket_close(fd);
            flb_error("[io] could not bind source_address=%s",
                      u->net.source_address);
            return -1;
        }
    }
    else {
        /* Create the socket */
        if (u_conn->u->flags & FLB_IO_IPV6) {
            fd = flb_net_socket_create(AF_INET6, async);
        }
        else {
            fd = flb_net_socket_create(AF_INET, async);
        }
        if (fd == -1) {
            flb_error("[io] could not create socket");
            return -1;
        }
    }

    u_conn->fd = fd;
    u_conn->event.fd = fd;

    /* Disable Nagle's algorithm */
    flb_net_socket_tcp_nodelay(fd);

    /* Connect */