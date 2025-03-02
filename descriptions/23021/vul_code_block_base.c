u_int name_offset, padding_offset = body_offset + 4;

		name_offset = padding_offset;
		for(i=0; i<14; i++) if(packet->payload[name_offset] != 0x1b) name_offset++; /* ASN.1 */

#ifdef KERBEROS_DEBUG
		printf("name_offset=%u [%02X %02X] [byte 0 must be 0x1b]\n", name_offset, packet->payload[name_offset], packet->payload[name_offset+1]);
#endif

		if(name_offset < (packet->payload_packet_len+1)) {
		  u_int realm_len;

		  name_offset++;
		  realm_len = packet->payload[name_offset];

		  if((realm_len+name_offset) < packet->payload_packet_len) {
		    char realm_str[48];

		    if(realm_len > sizeof(realm_str)-1)
		      realm_len = sizeof(realm_str)-1;

		    name_offset += 1;

		    strncpy(realm_str, (char*)&packet->payload[name_offset], realm_len);
		    realm_str[realm_len] = '\0';
		    for(i=0; i<realm_len; i++) realm_str[i] = tolower(realm_str[i]);

#ifdef KERBEROS_DEBUG
		    printf("[TGS-REQ][s/dport: %u/%u][Kerberos Realm][len: %u][%s]\n", sport, dport, realm_len, realm_str);
#endif
		    snprintf(flow->protos.kerberos.domain, sizeof(flow->protos.kerberos.domain), "%s", realm_str);

		    /* If necessary we can decode sname */
		    if(flow->kerberos_buf.pktbuf) {
			    ndpi_free(flow->kerberos_buf.pktbuf);
			    packet->payload = original_packet_payload;
			    packet->payload_packet_len = original_payload_packet_len;
		    }
		    flow->kerberos_buf.pktbuf = NULL;
		  }
		}