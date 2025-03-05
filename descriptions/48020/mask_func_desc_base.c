void process_ndpi_collected_info(struct ndpi_workflow * workflow, struct ndpi_flow_info *flow) {
  u_int i, is_quic = 0;
  char out[128], *s;
  
  if(!flow->ndpi_flow) return;

  flow->info_type = INFO_INVALID;

  s = ndpi_get_flow_risk_info(flow->ndpi_flow, out, sizeof(out), 0 /* text */);

  if(s != NULL)
    flow->risk_str = ndpi_strdup(s);  
  
  flow->confidence = flow->ndpi_flow->confidence;

  ndpi_snprintf(flow->host_server_name, sizeof(flow->host_server_name), "%s",
	   flow->ndpi_flow->host_server_name);

  ndpi_snprintf(flow->flow_extra_info, sizeof(flow->flow_extra_info), "%s",
	   flow->ndpi_flow->flow_extra_info);

  flow->risk = flow->ndpi_flow->risk;

  if(is_ndpi_proto(flow, NDPI_PROTOCOL_DHCP)) {
    if(flow->ndpi_flow->protos.dhcp.fingerprint[0] != '\0')
      flow->dhcp_fingerprint = ndpi_strdup(flow->ndpi_flow->protos.dhcp.fingerprint);
    if(flow->ndpi_flow->protos.dhcp.class_ident[0] != '\0')
      flow->dhcp_class_ident = ndpi_strdup(flow->ndpi_flow->protos.dhcp.class_ident);
  } else if(is_ndpi_proto(flow, NDPI_PROTOCOL_BITTORRENT) &&
            !is_ndpi_proto(flow, NDPI_PROTOCOL_TLS)) {
    u_int j;

    if(flow->ndpi_flow->protos.bittorrent.hash[0] != '\0') {
      flow->bittorent_hash = ndpi_malloc(sizeof(flow->ndpi_flow->protos.bittorrent.hash) * 2 + 1);
      for(i=0, j = 0; i < sizeof(flow->ndpi_flow->protos.bittorrent.hash); i++) {
        sprintf(&flow->bittorent_hash[j], "%02x",
	        flow->ndpi_flow->protos.bittorrent.hash[i]);

        j += 2;
      }
      flow->bittorent_hash[j] = '\0';
    }
  }
  /* DNS */
  else if(is_ndpi_proto(flow, NDPI_PROTOCOL_DNS)) {
    if(flow->ndpi_flow->protos.dns.rsp_type == 0x1)
    {
      flow->info_type = INFO_GENERIC;
      inet_ntop(AF_INET, &flow->ndpi_flow->protos.dns.rsp_addr.ipv4, flow->info, sizeof(flow->info));
    } else {
      flow->info_type = INFO_GENERIC;
      inet_ntop(AF_INET6, &flow->ndpi_flow->protos.dns.rsp_addr.ipv6, flow->info, sizeof(flow->info));

      /* For consistency across platforms replace :0: with :: */
      ndpi_patchIPv6Address(flow->info);
    }
  }
  /* MDNS */
  else if(is_ndpi_proto(flow, NDPI_PROTOCOL_MDNS)) {
    flow->info_type = INFO_GENERIC;
    ndpi_snprintf(flow->info, sizeof(flow->info), "%s", flow->ndpi_flow->host_server_name);
  }
  /* UBNTAC2 */
  else if(is_ndpi_proto(flow, NDPI_PROTOCOL_UBNTAC2)) {
    flow->info_type = INFO_GENERIC;
    ndpi_snprintf(flow->info, sizeof(flow->info), "%s", flow->ndpi_flow->protos.ubntac2.version);
  }
  /* FTP */
  else if((is_ndpi_proto(flow, NDPI_PROTOCOL_FTP_CONTROL))
	  || /* IMAP */ is_ndpi_proto(flow, NDPI_PROTOCOL_MAIL_IMAP)
	  || /* POP */  is_ndpi_proto(flow, NDPI_PROTOCOL_MAIL_POP)
	  || /* SMTP */ is_ndpi_proto(flow, NDPI_PROTOCOL_MAIL_SMTP)) {
    flow->info_type = INFO_FTP_IMAP_POP_SMTP;
    ndpi_snprintf(flow->ftp_imap_pop_smtp.username,
                  sizeof(flow->ftp_imap_pop_smtp.username),
                  "%s", flow->ndpi_flow->l4.tcp.ftp_imap_pop_smtp.username);
    ndpi_snprintf(flow->ftp_imap_pop_smtp.password,
                  sizeof(flow->ftp_imap_pop_smtp.password),
                  "%s", flow->ndpi_flow->l4.tcp.ftp_imap_pop_smtp.password);
    flow->ftp_imap_pop_smtp.auth_failed =
      flow->ndpi_flow->l4.tcp.ftp_imap_pop_smtp.auth_failed;
  }
  /* KERBEROS */
  else if(is_ndpi_proto(flow, NDPI_PROTOCOL_KERBEROS)) {
    flow->info_type = INFO_KERBEROS;
    ndpi_snprintf(flow->kerberos.domain,
                  sizeof(flow->kerberos.domain),
                  "%s", flow->ndpi_flow->protos.kerberos.domain);
    ndpi_snprintf(flow->kerberos.hostname,
                  sizeof(flow->kerberos.hostname),
                  "%s", flow->ndpi_flow->protos.kerberos.hostname);
    ndpi_snprintf(flow->kerberos.username,
                  sizeof(flow->kerberos.username),
                  "%s", flow->ndpi_flow->protos.kerberos.username);
  }
  /* HTTP */
  else if((flow->detected_protocol.master_protocol == NDPI_PROTOCOL_HTTP)
	  || is_ndpi_proto(flow, NDPI_PROTOCOL_HTTP)) {
    if(flow->ndpi_flow->http.url != NULL) {
      ndpi_snprintf(flow->http.url, sizeof(flow->http.url), "%s", flow->ndpi_flow->http.url);
      flow->http.response_status_code = flow->ndpi_flow->http.response_status_code;
      ndpi_snprintf(flow->http.content_type, sizeof(flow->http.content_type), "%s", flow->ndpi_flow->http.content_type ? flow->ndpi_flow->http.content_type : "");
      ndpi_snprintf(flow->http.request_content_type, sizeof(flow->http.request_content_type), "%s", flow->ndpi_flow->http.request_content_type ? flow->ndpi_flow->http.request_content_type : "");
    }
  } else if(is_ndpi_proto(flow, NDPI_PROTOCOL_TELNET)) {
    if(flow->ndpi_flow->protos.telnet.username[0] != '\0')
      flow->telnet.username = ndpi_strdup(flow->ndpi_flow->protos.telnet.username);
    if(flow->ndpi_flow->protos.telnet.password[0] != '\0')
      flow->telnet.password = ndpi_strdup(flow->ndpi_flow->protos.telnet.password);
  } else 
  // Check if the flow corresponds to the SSH protocol.
  // If so, copy SSH-related information, such as client and server signatures,
  // and client/server hash data to the flow structure.
  // 
  // Check if the flow corresponds to the TLS protocol or related protocols.
  // If the protocol matches, update the flow structure with relevant TLS/QUIC information.
  // This includes SSL version, server names, certificate validity dates, and JA3 fingerprints.
  // Extract and store SSL/TLS version, server names, and certificate information.
  // Copy various TLS/QUIC attributes, including ALPN and fingerprint data, to the flow.
  // 
  // If ALPN and supported versions are present, store them and set the appropriate info type.
  // Optionally, reset and update the payload length distribution based on TLS block lengths.
  // <MASK>

  ndpi_snprintf(flow->http.user_agent,
                sizeof(flow->http.user_agent),
                "%s", (flow->ndpi_flow->http.user_agent ? flow->ndpi_flow->http.user_agent : ""));

  if(flow->detection_completed && (!flow->check_extra_packets)) {
    if(is_ndpi_proto(flow, NDPI_PROTOCOL_UNKNOWN)) {
      if(workflow->__flow_giveup_callback != NULL)
	workflow->__flow_giveup_callback(workflow, flow, workflow->__flow_giveup_udata);
    } else {
      if(workflow->__flow_detected_callback != NULL)
	workflow->__flow_detected_callback(workflow, flow, workflow->__flow_detected_udata);
    }

    ndpi_free_flow_info_half(flow);
  }
}