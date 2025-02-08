ssl_common_register_ssl_alpn_dissector_table("tls.alpn",
        "SSL/TLS Application-Layer Protocol Negotiation (ALPN) Protocol IDs",
        proto_tls);

    tls_handle = register_dissector("tls", dissect_ssl, proto_tls);
    register_dissector("tls13-handshake", dissect_tls13_handshake, proto_tls);

    register_init_routine(ssl_init);