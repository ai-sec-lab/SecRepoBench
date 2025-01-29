if(flow->http.url)
      ndpi_free(flow->http.url);
    if(flow->http.content_type)
      ndpi_free(flow->http.content_type);
    if(flow->http.user_agent)
      ndpi_free(flow->http.user_agent);
    if(flow->kerberos_buf.pktbuf)
      ndpi_free(flow->kerberos_buf.pktbuf);

    if(flow_is_proto(flow, NDPI_PROTOCOL_TLS) ||
       flow_is_proto(flow, NDPI_PROTOCOL_QUIC)) {
      if(flow->protos.stun_ssl.ssl.server_names)
	ndpi_free(flow->protos.stun_ssl.ssl.server_names);

      if(flow->protos.stun_ssl.ssl.alpn)
	ndpi_free(flow->protos.stun_ssl.ssl.alpn);

      if(flow->protos.stun_ssl.ssl.tls_supported_versions)
	ndpi_free(flow->protos.stun_ssl.ssl.tls_supported_versions);

      if(flow->protos.stun_ssl.ssl.issuerDN)
	ndpi_free(flow->protos.stun_ssl.ssl.issuerDN);

      if(flow->protos.stun_ssl.ssl.subjectDN)
	ndpi_free(flow->protos.stun_ssl.ssl.subjectDN);

      if(flow->l4.tcp.tls.srv_cert_fingerprint_ctx)
	ndpi_free(flow->l4.tcp.tls.srv_cert_fingerprint_ctx);

      if(flow->protos.stun_ssl.ssl.encrypted_sni.esni)
	ndpi_free(flow->protos.stun_ssl.ssl.encrypted_sni.esni);
    }