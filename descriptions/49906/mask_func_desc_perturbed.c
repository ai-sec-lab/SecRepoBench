static void check_content_type_and_change_protocol(struct ndpi_detection_module_struct *ndpi_struct,
						   struct ndpi_flow_struct *http_flow) {
  struct ndpi_packet_struct *packet = &ndpi_struct->packet;
  int ret;

  if(http_flow->http_detected && (http_flow->http.response_status_code != 0))
    return;

  if((http_flow->http.url == NULL)
     && (packet->http_url_name.len > 0)
     && (packet->host_line.len > 0)) {
    int len = packet->http_url_name.len + packet->host_line.len + 1;

    if(isdigit(packet->host_line.ptr[0])
       && (packet->host_line.len < 21))
      ndpi_check_numeric_ip(ndpi_struct, http_flow, (char*)packet->host_line.ptr, packet->host_line.len);

    http_flow->http.url = ndpi_malloc(len);
    if(http_flow->http.url) {
      u_int offset = 0;

      if(http_flow->detected_protocol_stack[0] == NDPI_PROTOCOL_HTTP_CONNECT) {
	strncpy(http_flow->http.url, (char*)packet->http_url_name.ptr,
		packet->http_url_name.len);

	http_flow->http.url[packet->http_url_name.len] = '\0';
      } else {
	/* Check if we pass through a proxy (usually there is also the Via: ... header) */
	if(strncmp((char*)packet->http_url_name.ptr, "http://", 7) != 0)
	  strncpy(http_flow->http.url, (char*)packet->host_line.ptr, offset = packet->host_line.len);

	if((packet->host_line.len == packet->http_url_name.len)
	   && (strncmp((char*)packet->host_line.ptr,
		       (char*)packet->http_url_name.ptr, packet->http_url_name.len) == 0))
	  ;
	else {
	  strncpy(&http_flow->http.url[offset], (char*)packet->http_url_name.ptr,
		  packet->http_url_name.len);
	  offset += packet->http_url_name.len;
	}

	http_flow->http.url[offset] = '\0';
      }

      ndpi_check_http_url(ndpi_struct, http_flow, &http_flow->http.url[packet->host_line.len]);
    }

    http_flow->http.method = ndpi_http_str2method((const char*)packet->http_method.ptr,
					     (u_int16_t)packet->http_method.len);

    if((http_flow->http.method == NDPI_HTTP_METHOD_RPC_IN_DATA)
       || (http_flow->http.method == NDPI_HTTP_METHOD_RPC_OUT_DATA)) {
      ndpi_set_detected_protocol(ndpi_struct, http_flow, NDPI_PROTOCOL_RPC, http_flow->detected_protocol_stack[0], NDPI_CONFIDENCE_DPI);
      check_content_type_and_change_protocol(ndpi_struct, http_flow);
    }
  }

  if(packet->server_line.ptr != NULL && (packet->server_line.len > 7)) {
    if(strncmp((const char *)packet->server_line.ptr, "ntopng ", 7) == 0) {
      ndpi_set_detected_protocol(ndpi_struct, http_flow, NDPI_PROTOCOL_NTOP, NDPI_PROTOCOL_HTTP, NDPI_CONFIDENCE_DPI);
      NDPI_CLR_BIT(http_flow->risk, NDPI_KNOWN_PROTOCOL_ON_NON_STANDARD_PORT);
    }
  }

  if(packet->user_agent_line.ptr != NULL && packet->user_agent_line.len != 0) {
    ret = http_process_user_agent(ndpi_struct, http_flow, packet->user_agent_line.ptr, packet->user_agent_line.len);
    /* TODO: Is it correct to avoid setting ua, host_name,... if we have a (Netflix) subclassification? */
    if(ret != 0)
      return;
  }

  /* check for host line */
  if(packet->host_line.ptr != NULL) {
    u_int len;

    NDPI_LOG_DBG2(ndpi_struct, "HOST line found %.*s\n",
		  packet->host_line.len, packet->host_line.ptr);

    /* Copy result for nDPI apps */
    ndpi_hostname_sni_set(http_flow, packet->host_line.ptr, packet->host_line.len);

    if(strlen(http_flow->host_server_name) > 0) {
      ndpi_check_dga_name(ndpi_struct, http_flow, http_flow->host_server_name, 1);

      if(ndpi_is_valid_hostname(http_flow->host_server_name,
				strlen(http_flow->host_server_name)) == 0) {
	char str[128];

	snprintf(str, sizeof(str), "Invalid host %s", http_flow->host_server_name);
	ndpi_set_risk(ndpi_struct, http_flow, NDPI_INVALID_CHARACTERS, str);

	/* This looks like an attack */
	ndpi_set_risk(ndpi_struct, http_flow, NDPI_POSSIBLE_EXPLOIT, NULL);
      }
    }

    if(packet->forwarded_line.ptr) {
      if(http_flow->http.nat_ip == NULL) {
        len = packet->forwarded_line.len;
        http_flow->http.nat_ip = ndpi_malloc(len + 1);
        if(http_flow->http.nat_ip != NULL) {
          strncpy(http_flow->http.nat_ip, (char*)packet->forwarded_line.ptr, len);
          http_flow->http.nat_ip[len] = '\0';
        }
      }
    }

    ndpi_http_parse_subprotocol(ndpi_struct, http_flow);

    /**
       check result of host subprotocol detection

       if "detected" in flow == 0 then "detected" = "guess"
       else "guess" = "detected"
    **/
    if
    // Update the protocol detection stack in the flow structure. If the second protocol
    // in the stack is unknown, and the current guessed protocol has no subprotocols,
    // set the first protocol in the stack to the guessed host protocol, and the second
    // to the guessed protocol. If the second protocol is already detected, ensure that
    // the guessed protocol and guessed host protocol match the detected protocols in the stack.
    // <MASK>

    if((http_flow->detected_protocol_stack[0] == NDPI_PROTOCOL_UNKNOWN)
       && (http_flow->http_detected)
       && (packet->http_origin.len > 0)) {
      ndpi_protocol_match_result ret_match;

      ndpi_match_host_subprotocol(ndpi_struct, http_flow,
				  (char *)packet->http_origin.ptr,
				  packet->http_origin.len,
				  &ret_match,
				  NDPI_PROTOCOL_HTTP);
    }

    if(http_flow->detected_protocol_stack[1] != NDPI_PROTOCOL_UNKNOWN) {
      if((http_flow->detected_protocol_stack[1] != NDPI_PROTOCOL_HTTP)
	 && (http_flow->detected_protocol_stack[1] != NDPI_PROTOCOL_HTTP_CONNECT)
	 && (http_flow->detected_protocol_stack[1] != NDPI_PROTOCOL_HTTP_PROXY)) {
	NDPI_LOG_INFO(ndpi_struct, "found HTTP/%s\n",
		      ndpi_get_proto_name(ndpi_struct, http_flow->detected_protocol_stack[0]));
	ndpi_int_http_add_connection(ndpi_struct, http_flow, http_flow->detected_protocol_stack[0], NDPI_PROTOCOL_CATEGORY_WEB);
	return; /* We have identified a sub-protocol so we're done */
      }
    }
  }

  if(http_flow->guessed_protocol_id == NDPI_PROTOCOL_UNKNOWN)
    http_flow->guessed_protocol_id = NDPI_PROTOCOL_HTTP;

  /* check for accept line */
  if(packet->accept_line.ptr != NULL) {
    NDPI_LOG_DBG2(ndpi_struct, "Accept line found %.*s\n",
		  packet->accept_line.len, packet->accept_line.ptr);
  }

  /* check for authorization line */
  if(packet->authorization_line.ptr != NULL) {
    NDPI_LOG_DBG2(ndpi_struct, "Authorization line found %.*s\n",
		  packet->authorization_line.len, packet->authorization_line.ptr);

    if(ndpi_strncasestr((const char*)packet->authorization_line.ptr,
			"Basic", packet->authorization_line.len)
       || ndpi_strncasestr((const char*)packet->authorization_line.ptr,
			   "Digest", packet->authorization_line.len)) {
      ndpi_set_risk(ndpi_struct, http_flow, NDPI_CLEAR_TEXT_CREDENTIALS,
		    "Found credentials in HTTP Auth Line");
    }
  }

  if(packet->content_line.ptr != NULL && packet->content_line.len != 0) {
    NDPI_LOG_DBG2(ndpi_struct, "Content Type line found %.*s\n",
		  packet->content_line.len, packet->content_line.ptr);

    if(http_flow->http.response_status_code == 0) {
      /* Request */
      if((http_flow->http.request_content_type == NULL) && (packet->content_line.len > 0)) {
	int len = packet->content_line.len + 1;

	http_flow->http.request_content_type = ndpi_malloc(len);
	if(http_flow->http.request_content_type) {
	  strncpy(http_flow->http.request_content_type, (char*)packet->content_line.ptr,
		  packet->content_line.len);
	  http_flow->http.request_content_type[packet->content_line.len] = '\0';
	}
      }
    } else {
      /* Response */
      if((http_flow->http.content_type == NULL) && (packet->content_line.len > 0)) {
	int len = packet->content_line.len + 1;

	http_flow->http.content_type = ndpi_malloc(len);
	if(http_flow->http.content_type) {
	  strncpy(http_flow->http.content_type, (char*)packet->content_line.ptr,
		  packet->content_line.len);
	  http_flow->http.content_type[packet->content_line.len] = '\0';

	  http_flow->guessed_category = http_flow->category = ndpi_http_check_content(ndpi_struct, http_flow);
	}
      }
    }

    if(http_flow->http_detected && packet->content_line.ptr && *(char*)packet->content_line.ptr) {
      /* Matching on Content-Type.
          OCSP:  application/ocsp-request, application/ocsp-response
       */
      if(strncmp((const char *)packet->content_line.ptr, "application/ocsp-", 17) == 0) {
        NDPI_LOG_DBG2(ndpi_struct, "Found OCSP\n");
        ndpi_set_detected_protocol(ndpi_struct, http_flow, NDPI_PROTOCOL_OCSP, NDPI_PROTOCOL_HTTP, NDPI_CONFIDENCE_DPI);
      }
    }
  }

  if(packet->user_agent_line.ptr != NULL && packet->user_agent_line.len != 0 &&
     http_flow->http.url) {
    /* WindowsUpdate over some kind of CDN */
    if(http_flow->detected_protocol_stack[1] == NDPI_PROTOCOL_UNKNOWN &&
       http_flow->detected_protocol_stack[0] == NDPI_PROTOCOL_HTTP &&
       (strstr(http_flow->http.url, "delivery.mp.microsoft.com/") ||
        strstr(http_flow->http.url, "download.windowsupdate.com/")) &&
       ndpi_strnstr((const char *)packet->user_agent_line.ptr, "Microsoft-Delivery-Optimization/",
                    packet->user_agent_line.len) &&
       ndpi_isset_risk(ndpi_struct, http_flow, NDPI_HTTP_NUMERIC_IP_HOST)) {
      ndpi_set_detected_protocol(ndpi_struct, http_flow, NDPI_PROTOCOL_WINDOWS_UPDATE, NDPI_PROTOCOL_HTTP, NDPI_CONFIDENCE_DPI);
    }
  }

  if(ndpi_get_http_method(ndpi_struct, http_flow) != NDPI_HTTP_METHOD_UNKNOWN) {
    ndpi_int_http_add_connection(ndpi_struct, http_flow, http_flow->detected_protocol_stack[0], NDPI_PROTOCOL_CATEGORY_WEB);
  }
}