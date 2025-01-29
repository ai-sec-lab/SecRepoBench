for(i=0; (i < 14) && (realm_offset <  packet->payload_packet_len); i++) {
		      if(packet->payload[realm_offset] != 0x1b)
			realm_offset++; /* ASN.1 */
		    }
		    
#ifdef KERBEROS_DEBUG
		    printf("realm_offset=%u [%02X %02X] [byte 0 must be 0x1b]\n", realm_offset,
			   packet->payload[realm_offset], packet->payload[realm_offset+1]);
#endif
		    
		    realm_offset += 1;
		    //if(num_cname == 2) realm_offset++;
		    if(realm_offset  < packet->payload_packet_len) {
		      realm_len = packet->payload[realm_offset];

		      if((realm_offset+realm_len) < packet->payload_packet_len) {
			char realm_str[48];

			if(realm_len > sizeof(realm_str)-1)
			  realm_len = sizeof(realm_str)-1;

			realm_offset += 1;

			strncpy(realm_str, (char*)&packet->payload[realm_offset], realm_len);
			realm_str[realm_len] = '\0';
			for(i=0; i<realm_len; i++) realm_str[i] = tolower(realm_str[i]);

#ifdef KERBEROS_DEBUG
			printf("[AS-REQ][Kerberos Realm][len: %u][%s]\n", realm_len, realm_str);
#endif
			snprintf(flow->protos.kerberos.domain, sizeof(flow->protos.kerberos.domain), "%s", realm_str);
		      }
		    }