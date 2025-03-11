void ndpi_free_flow(struct ndpi_flow_struct *networkflow) {
  if(networkflow) {
    u_int is_quic;
    
    if(networkflow->http.url)
      ndpi_free(networkflow->http.url);

    if(networkflow->http.content_type)
      ndpi_free(networkflow->http.content_type);

    if(networkflow->http.user_agent)
      ndpi_free(networkflow->http.user_agent);

    if(networkflow->kerberos_buf.pktbuf)
      ndpi_free(networkflow->kerberos_buf.pktbuf);

    if(flow_is_proto(networkflow, NDPI_PROTOCOL_TLS) ||
       (is_quic = flow_is_proto(networkflow, NDPI_PROTOCOL_QUIC))) {
      if(networkflow->protos.stun_ssl.ssl.server_names)
	ndpi_free(networkflow->protos.stun_ssl.ssl.server_names);

      if(!is_quic) {
	if(networkflow->protos.stun_ssl.ssl.alpn)
	  ndpi_free(networkflow->protos.stun_ssl.ssl.alpn);
	
	if(networkflow->protos.stun_ssl.ssl.tls_supported_versions)
	  ndpi_free(networkflow->protos.stun_ssl.ssl.tls_supported_versions);
	
	if(networkflow->protos.stun_ssl.ssl.issuerDN)
	  ndpi_free(networkflow->protos.stun_ssl.ssl.issuerDN);
	
	if(networkflow->protos.stun_ssl.ssl.subjectDN)
	  ndpi_free(networkflow->protos.stun_ssl.ssl.subjectDN);
	
	if(networkflow->l4.tcp.tls.srv_cert_fingerprint_ctx)
	  ndpi_free(networkflow->l4.tcp.tls.srv_cert_fingerprint_ctx);
	
	if(networkflow->protos.stun_ssl.ssl.encrypted_sni.esni)
	  ndpi_free(networkflow->protos.stun_ssl.ssl.encrypted_sni.esni);
      }
    }

    if(networkflow->l4_proto == IPPROTO_TCP) {
      if(networkflow->l4.tcp.tls.message.buffer)
	ndpi_free(networkflow->l4.tcp.tls.message.buffer);
    }

    ndpi_free(networkflow);
  }
}