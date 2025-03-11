if(is_ndpi_proto(flow_info, NDPI_PROTOCOL_SSH)) {
    ndpi_snprintf(flow_info->host_server_name,
	     sizeof(flow_info->host_server_name), "%s",
	     flow_info->ndpi_flow->protos.ssh.client_signature);
    ndpi_snprintf(flow_info->ssh_tls.server_info, sizeof(flow_info->ssh_tls.server_info), "%s",
	     flow_info->ndpi_flow->protos.ssh.server_signature);
    ndpi_snprintf(flow_info->ssh_tls.client_hassh, sizeof(flow_info->ssh_tls.client_hassh), "%s",
	     flow_info->ndpi_flow->protos.ssh.hassh_client);
    ndpi_snprintf(flow_info->ssh_tls.server_hassh, sizeof(flow_info->ssh_tls.server_hassh), "%s",
	     flow_info->ndpi_flow->protos.ssh.hassh_server);
  }
  /* TLS */
  else if((is_ndpi_proto(flow_info, NDPI_PROTOCOL_TLS))
	  || ((is_quic = is_ndpi_proto(flow_info, NDPI_PROTOCOL_QUIC)))
	  || (flow_info->detected_protocol.master_protocol == NDPI_PROTOCOL_TLS)
	  || (flow_info->ndpi_flow->protos.tls_quic.ja3_client[0] != '\0')
	  ) {
    flow_info->ssh_tls.ssl_version = flow_info->ndpi_flow->protos.tls_quic.ssl_version;

    if(flow_info->ndpi_flow->protos.tls_quic.server_names_len > 0 && flow_info->ndpi_flow->protos.tls_quic.server_names)
      flow_info->ssh_tls.server_names = ndpi_strdup(flow_info->ndpi_flow->protos.tls_quic.server_names);

    flow_info->ssh_tls.notBefore = flow_info->ndpi_flow->protos.tls_quic.notBefore;
    flow_info->ssh_tls.notAfter = flow_info->ndpi_flow->protos.tls_quic.notAfter;
    ndpi_snprintf(flow_info->ssh_tls.ja3_client, sizeof(flow_info->ssh_tls.ja3_client), "%s",
	     flow_info->ndpi_flow->protos.tls_quic.ja3_client);
    ndpi_snprintf(flow_info->ssh_tls.ja3_server, sizeof(flow_info->ssh_tls.ja3_server), "%s",
	     flow_info->ndpi_flow->protos.tls_quic.ja3_server);
    flow_info->ssh_tls.server_unsafe_cipher = flow_info->ndpi_flow->protos.tls_quic.server_unsafe_cipher;
    flow_info->ssh_tls.server_cipher = flow_info->ndpi_flow->protos.tls_quic.server_cipher;

    if(flow_info->ndpi_flow->l4.tcp.tls.fingerprint_set) {
      memcpy(flow_info->ssh_tls.sha1_cert_fingerprint,
	     flow_info->ndpi_flow->protos.tls_quic.sha1_certificate_fingerprint, 20);
      flow_info->ssh_tls.sha1_cert_fingerprint_set = 1;
    }

    flow_info->ssh_tls.browser_heuristics = flow_info->ndpi_flow->protos.tls_quic.browser_heuristics;

    if(flow_info->ndpi_flow->protos.tls_quic.alpn) {
      if((flow_info->ssh_tls.tls_alpn = ndpi_strdup(flow_info->ndpi_flow->protos.tls_quic.alpn)) != NULL)
	correct_csv_data_field(flow_info->ssh_tls.tls_alpn);
    }

    if(flow_info->ndpi_flow->protos.tls_quic.issuerDN)
      flow_info->ssh_tls.tls_issuerDN = strdup(flow_info->ndpi_flow->protos.tls_quic.issuerDN);

    if(flow_info->ndpi_flow->protos.tls_quic.subjectDN)
      flow_info->ssh_tls.tls_subjectDN = strdup(flow_info->ndpi_flow->protos.tls_quic.subjectDN);

    if(flow_info->ndpi_flow->protos.tls_quic.encrypted_sni.esni) {
      flow_info->ssh_tls.encrypted_sni.esni = strdup(flow_info->ndpi_flow->protos.tls_quic.encrypted_sni.esni);
      flow_info->ssh_tls.encrypted_sni.cipher_suite = flow_info->ndpi_flow->protos.tls_quic.encrypted_sni.cipher_suite;
    }

    if(flow_info->ssh_tls.tls_supported_versions) {
      if((flow_info->ssh_tls.tls_supported_versions = ndpi_strdup(flow_info->ndpi_flow->protos.tls_quic.tls_supported_versions)) != NULL)
	correct_csv_data_field(flow_info->ssh_tls.tls_supported_versions);
    }

    if(flow_info->ndpi_flow->protos.tls_quic.alpn
       && flow_info->ndpi_flow->protos.tls_quic.tls_supported_versions) {
      correct_csv_data_field(flow_info->ndpi_flow->protos.tls_quic.alpn);
      correct_csv_data_field(flow_info->ndpi_flow->protos.tls_quic.tls_supported_versions);

      flow_info->info_type = INFO_TLS_QUIC_ALPN_VERSION;
      ndpi_snprintf(flow_info->tls_quic.alpn, sizeof(flow_info->tls_quic.alpn), "%s",
                    flow_info->ndpi_flow->protos.tls_quic.alpn);
      ndpi_snprintf(flow_info->tls_quic.tls_supported_versions,
                    sizeof(flow_info->tls_quic.tls_supported_versions),
                    "%s", flow_info->ndpi_flow->protos.tls_quic.tls_supported_versions);
    } else if(flow_info->ndpi_flow->protos.tls_quic.alpn) {
      correct_csv_data_field(flow_info->ndpi_flow->protos.tls_quic.alpn);

      flow_info->info_type = INFO_TLS_QUIC_ALPN_ONLY;
      ndpi_snprintf(flow_info->tls_quic.alpn, sizeof(flow_info->tls_quic.alpn), "%s",
                    flow_info->ndpi_flow->protos.tls_quic.alpn);
    }

    if(enable_doh_dot_detection) {
      /* For TLS we use TLS block lenght instead of payload lenght */
      ndpi_reset_bin(&flow_info->payload_len_bin);

      for(i=0; i<flow_info->ndpi_flow->l4.tcp.tls.num_tls_blocks; i++) {
	u_int16_t len = abs(flow_info->ndpi_flow->l4.tcp.tls.tls_application_blocks_len[i]);

	/* printf("[TLS_LEN] %u\n", len); */
	ndpi_inc_bin(&flow_info->payload_len_bin, plen2slot(len), 1);
      }
    }
  }