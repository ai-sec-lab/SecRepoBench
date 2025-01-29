if(is_ndpi_proto(flow, NDPI_PROTOCOL_SSH)) {
    ndpi_snprintf(flow->host_server_name,
	     sizeof(flow->host_server_name), "%s",
	     flow->ndpi_flow->protos.ssh.client_signature);
    ndpi_snprintf(flow->ssh_tls.server_info, sizeof(flow->ssh_tls.server_info), "%s",
	     flow->ndpi_flow->protos.ssh.server_signature);
    ndpi_snprintf(flow->ssh_tls.client_hassh, sizeof(flow->ssh_tls.client_hassh), "%s",
	     flow->ndpi_flow->protos.ssh.hassh_client);
    ndpi_snprintf(flow->ssh_tls.server_hassh, sizeof(flow->ssh_tls.server_hassh), "%s",
	     flow->ndpi_flow->protos.ssh.hassh_server);
  }
  /* TLS */
  else if((is_ndpi_proto(flow, NDPI_PROTOCOL_TLS))
	  || ((is_quic = is_ndpi_proto(flow, NDPI_PROTOCOL_QUIC)))
	  || (flow->detected_protocol.master_protocol == NDPI_PROTOCOL_TLS)
	  || (flow->ndpi_flow->protos.tls_quic.ja3_client[0] != '\0')
	  ) {
    flow->ssh_tls.ssl_version = flow->ndpi_flow->protos.tls_quic.ssl_version;

    if(flow->ndpi_flow->protos.tls_quic.server_names_len > 0 && flow->ndpi_flow->protos.tls_quic.server_names)
      flow->ssh_tls.server_names = ndpi_strdup(flow->ndpi_flow->protos.tls_quic.server_names);

    flow->ssh_tls.notBefore = flow->ndpi_flow->protos.tls_quic.notBefore;
    flow->ssh_tls.notAfter = flow->ndpi_flow->protos.tls_quic.notAfter;
    ndpi_snprintf(flow->ssh_tls.ja3_client, sizeof(flow->ssh_tls.ja3_client), "%s",
	     flow->ndpi_flow->protos.tls_quic.ja3_client);
    ndpi_snprintf(flow->ssh_tls.ja3_server, sizeof(flow->ssh_tls.ja3_server), "%s",
	     flow->ndpi_flow->protos.tls_quic.ja3_server);
    flow->ssh_tls.server_unsafe_cipher = flow->ndpi_flow->protos.tls_quic.server_unsafe_cipher;
    flow->ssh_tls.server_cipher = flow->ndpi_flow->protos.tls_quic.server_cipher;

    if(flow->ndpi_flow->l4.tcp.tls.fingerprint_set) {
      memcpy(flow->ssh_tls.sha1_cert_fingerprint,
	     flow->ndpi_flow->protos.tls_quic.sha1_certificate_fingerprint, 20);
      flow->ssh_tls.sha1_cert_fingerprint_set = 1;
    }

    flow->ssh_tls.browser_heuristics = flow->ndpi_flow->protos.tls_quic.browser_heuristics;

    if(flow->ndpi_flow->protos.tls_quic.alpn) {
      if((flow->ssh_tls.tls_alpn = ndpi_strdup(flow->ndpi_flow->protos.tls_quic.alpn)) != NULL)
	correct_csv_data_field(flow->ssh_tls.tls_alpn);
    }

    if(flow->ndpi_flow->protos.tls_quic.issuerDN)
      flow->ssh_tls.tls_issuerDN = strdup(flow->ndpi_flow->protos.tls_quic.issuerDN);

    if(flow->ndpi_flow->protos.tls_quic.subjectDN)
      flow->ssh_tls.tls_subjectDN = strdup(flow->ndpi_flow->protos.tls_quic.subjectDN);

    if(flow->ndpi_flow->protos.tls_quic.encrypted_sni.esni) {
      flow->ssh_tls.encrypted_sni.esni = strdup(flow->ndpi_flow->protos.tls_quic.encrypted_sni.esni);
      flow->ssh_tls.encrypted_sni.cipher_suite = flow->ndpi_flow->protos.tls_quic.encrypted_sni.cipher_suite;
    }

    if(flow->ssh_tls.tls_supported_versions) {
      if((flow->ssh_tls.tls_supported_versions = ndpi_strdup(flow->ndpi_flow->protos.tls_quic.tls_supported_versions)) != NULL)
	correct_csv_data_field(flow->ssh_tls.tls_supported_versions);
    }

    if(flow->ndpi_flow->protos.tls_quic.alpn
       && flow->ndpi_flow->protos.tls_quic.tls_supported_versions) {
      correct_csv_data_field(flow->ndpi_flow->protos.tls_quic.alpn);
      correct_csv_data_field(flow->ndpi_flow->protos.tls_quic.tls_supported_versions);

      flow->info_type = INFO_TLS_QUIC_ALPN_VERSION;
      ndpi_snprintf(flow->tls_quic.alpn, sizeof(flow->tls_quic.alpn), "%s",
                    flow->ndpi_flow->protos.tls_quic.alpn);
      ndpi_snprintf(flow->tls_quic.tls_supported_versions,
                    sizeof(flow->tls_quic.tls_supported_versions),
                    "%s", flow->ndpi_flow->protos.tls_quic.tls_supported_versions);
    } else if(flow->ndpi_flow->protos.tls_quic.alpn) {
      correct_csv_data_field(flow->ndpi_flow->protos.tls_quic.alpn);

      flow->info_type = INFO_TLS_QUIC_ALPN_ONLY;
      ndpi_snprintf(flow->tls_quic.alpn, sizeof(flow->tls_quic.alpn), "%s",
                    flow->ndpi_flow->protos.tls_quic.alpn);
    }

    if(enable_doh_dot_detection) {
      /* For TLS we use TLS block lenght instead of payload lenght */
      ndpi_reset_bin(&flow->payload_len_bin);

      for(i=0; i<flow->ndpi_flow->l4.tcp.tls.num_tls_blocks; i++) {
	u_int16_t len = abs(flow->ndpi_flow->l4.tcp.tls.tls_application_blocks_len[i]);

	/* printf("[TLS_LEN] %u\n", len); */
	ndpi_inc_bin(&flow->payload_len_bin, plen2slot(len), 1);
      }
    }
  }