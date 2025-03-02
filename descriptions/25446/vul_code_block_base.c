realm_offset = cname_len + name_offset + 3;

		    /* if cname does not end with a $ then it's a username */
		    if(cname_len && cname_str[cname_len-1] == '$') {
		      cname_str[cname_len-1] = '\0';
		      snprintf(flow->protos.kerberos.hostname, sizeof(flow->protos.kerberos.hostname), "%s", cname_str);
		    } else
		      snprintf(flow->protos.kerberos.username, sizeof(flow->protos.kerberos.username), "%s", cname_str);

		    for(i=0; (i < 14) && (realm_offset <  packet->payload_packet_len); i++) {
		      if(packet->payload[realm_offset] != 0x1b)
			realm_offset++; /* ASN.1 */
		    }