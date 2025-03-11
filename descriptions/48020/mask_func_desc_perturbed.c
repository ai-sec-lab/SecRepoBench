void process_ndpi_collected_info(struct ndpi_workflow * workflow, struct ndpi_flow_info *flow_info) {
  u_int i, is_quic = 0;
  char out[128], *s;
  
  if(!flow_info->ndpi_flow) return;

  flow_info->info_type = INFO_INVALID;

  s = ndpi_get_flow_risk_info(flow_info->ndpi_flow, out, sizeof(out), 0 /* text */);

  if(s != NULL)
    flow_info->risk_str = ndpi_strdup(s);  
  
  flow_info->confidence = flow_info->ndpi_flow->confidence;

  ndpi_snprintf(flow_info->host_server_name, sizeof(flow_info->host_server_name), "%s",
	   flow_info->ndpi_flow->host_server_name);

  ndpi_snprintf(flow_info->flow_extra_info, sizeof(flow_info->flow_extra_info), "%s",
	   flow_info->ndpi_flow->flow_extra_info);

  flow_info->risk = flow_info->ndpi_flow->risk;

  if(is_ndpi_proto(flow_info, NDPI_PROTOCOL_DHCP)) {
    if(flow_info->ndpi_flow->protos.dhcp.fingerprint[0] != '\0')
      flow_info->dhcp_fingerprint = ndpi_strdup(flow_info->ndpi_flow->protos.dhcp.fingerprint);
    if(flow_info->ndpi_flow->protos.dhcp.class_ident[0] != '\0')
      flow_info->dhcp_class_ident = ndpi_strdup(flow_info->ndpi_flow->protos.dhcp.class_ident);
  } else if(is_ndpi_proto(flow_info, NDPI_PROTOCOL_BITTORRENT) &&
            !is_ndpi_proto(flow_info, NDPI_PROTOCOL_TLS)) {
    u_int j;

    if(flow_info->ndpi_flow->protos.bittorrent.hash[0] != '\0') {
      flow_info->bittorent_hash = ndpi_malloc(sizeof(flow_info->ndpi_flow->protos.bittorrent.hash) * 2 + 1);
      for(i=0, j = 0; i < sizeof(flow_info->ndpi_flow->protos.bittorrent.hash); i++) {
        sprintf(&flow_info->bittorent_hash[j], "%02x",
	        flow_info->ndpi_flow->protos.bittorrent.hash[i]);

        j += 2;
      }
      flow_info->bittorent_hash[j] = '\0';
    }
  }
  /* DNS */
  else if(is_ndpi_proto(flow_info, NDPI_PROTOCOL_DNS)) {
    if(flow_info->ndpi_flow->protos.dns.rsp_type == 0x1)
    {
      flow_info->info_type = INFO_GENERIC;
      inet_ntop(AF_INET, &flow_info->ndpi_flow->protos.dns.rsp_addr.ipv4, flow_info->info, sizeof(flow_info->info));
    } else {
      flow_info->info_type = INFO_GENERIC;
      inet_ntop(AF_INET6, &flow_info->ndpi_flow->protos.dns.rsp_addr.ipv6, flow_info->info, sizeof(flow_info->info));

      /* For consistency across platforms replace :0: with :: */
      ndpi_patchIPv6Address(flow_info->info);
    }
  }
  /* MDNS */
  else if(is_ndpi_proto(flow_info, NDPI_PROTOCOL_MDNS)) {
    flow_info->info_type = INFO_GENERIC;
    ndpi_snprintf(flow_info->info, sizeof(flow_info->info), "%s", flow_info->ndpi_flow->host_server_name);
  }
  /* UBNTAC2 */
  else if(is_ndpi_proto(flow_info, NDPI_PROTOCOL_UBNTAC2)) {
    flow_info->info_type = INFO_GENERIC;
    ndpi_snprintf(flow_info->info, sizeof(flow_info->info), "%s", flow_info->ndpi_flow->protos.ubntac2.version);
  }
  /* FTP */
  else if((is_ndpi_proto(flow_info, NDPI_PROTOCOL_FTP_CONTROL))
	  || /* IMAP */ is_ndpi_proto(flow_info, NDPI_PROTOCOL_MAIL_IMAP)
	  || /* POP */  is_ndpi_proto(flow_info, NDPI_PROTOCOL_MAIL_POP)
	  || /* SMTP */ is_ndpi_proto(flow_info, NDPI_PROTOCOL_MAIL_SMTP)) {
    flow_info->info_type = INFO_FTP_IMAP_POP_SMTP;
    ndpi_snprintf(flow_info->ftp_imap_pop_smtp.username,
                  sizeof(flow_info->ftp_imap_pop_smtp.username),
                  "%s", flow_info->ndpi_flow->l4.tcp.ftp_imap_pop_smtp.username);
    ndpi_snprintf(flow_info->ftp_imap_pop_smtp.password,
                  sizeof(flow_info->ftp_imap_pop_smtp.password),
                  "%s", flow_info->ndpi_flow->l4.tcp.ftp_imap_pop_smtp.password);
    flow_info->ftp_imap_pop_smtp.auth_failed =
      flow_info->ndpi_flow->l4.tcp.ftp_imap_pop_smtp.auth_failed;
  }
  /* KERBEROS */
  else if(is_ndpi_proto(flow_info, NDPI_PROTOCOL_KERBEROS)) {
    flow_info->info_type = INFO_KERBEROS;
    ndpi_snprintf(flow_info->kerberos.domain,
                  sizeof(flow_info->kerberos.domain),
                  "%s", flow_info->ndpi_flow->protos.kerberos.domain);
    ndpi_snprintf(flow_info->kerberos.hostname,
                  sizeof(flow_info->kerberos.hostname),
                  "%s", flow_info->ndpi_flow->protos.kerberos.hostname);
    ndpi_snprintf(flow_info->kerberos.username,
                  sizeof(flow_info->kerberos.username),
                  "%s", flow_info->ndpi_flow->protos.kerberos.username);
  }
  /* HTTP */
  else if((flow_info->detected_protocol.master_protocol == NDPI_PROTOCOL_HTTP)
	  || is_ndpi_proto(flow_info, NDPI_PROTOCOL_HTTP)) {
    if(flow_info->ndpi_flow->http.url != NULL) {
      ndpi_snprintf(flow_info->http.url, sizeof(flow_info->http.url), "%s", flow_info->ndpi_flow->http.url);
      flow_info->http.response_status_code = flow_info->ndpi_flow->http.response_status_code;
      ndpi_snprintf(flow_info->http.content_type, sizeof(flow_info->http.content_type), "%s", flow_info->ndpi_flow->http.content_type ? flow_info->ndpi_flow->http.content_type : "");
      ndpi_snprintf(flow_info->http.request_content_type, sizeof(flow_info->http.request_content_type), "%s", flow_info->ndpi_flow->http.request_content_type ? flow_info->ndpi_flow->http.request_content_type : "");
    }
  } else if(is_ndpi_proto(flow_info, NDPI_PROTOCOL_TELNET)) {
    if(flow_info->ndpi_flow->protos.telnet.username[0] != '\0')
      flow_info->telnet.username = ndpi_strdup(flow_info->ndpi_flow->protos.telnet.username);
    if(flow_info->ndpi_flow->protos.telnet.password[0] != '\0')
      flow_info->telnet.password = ndpi_strdup(flow_info->ndpi_flow->protos.telnet.password);
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

  ndpi_snprintf(flow_info->http.user_agent,
                sizeof(flow_info->http.user_agent),
                "%s", (flow_info->ndpi_flow->http.user_agent ? flow_info->ndpi_flow->http.user_agent : ""));

  if(flow_info->detection_completed && (!flow_info->check_extra_packets)) {
    if(is_ndpi_proto(flow_info, NDPI_PROTOCOL_UNKNOWN)) {
      if(workflow->__flow_giveup_callback != NULL)
	workflow->__flow_giveup_callback(workflow, flow_info, workflow->__flow_giveup_udata);
    } else {
      if(workflow->__flow_detected_callback != NULL)
	workflow->__flow_detected_callback(workflow, flow_info, workflow->__flow_detected_udata);
    }

    ndpi_free_flow_info_half(flow_info);
  }
}