if(tot_alpn_len > packet->payload_packet_len)
	    return 0;

	  alpn_str[0] = '\0';
	  while(s_offset < tot_alpn_len && s_offset < total_len) {
	    u_int8_t alpn_i, alpn_len = packet->payload[s_offset++];

	    if((s_offset + alpn_len) <= tot_alpn_len) {
#ifdef DEBUG_TLS
	      printf("Server TLS [ALPN: %u]\n", alpn_len);
#endif

	      if(((uint32_t)alpn_str_len+alpn_len+1) < (sizeof(alpn_str)-1)) {
	        if(alpn_str_len > 0) {
	          alpn_str[alpn_str_len] = ',';
	          alpn_str_len++;
	        }

	        for(alpn_i=0; alpn_i<alpn_len; alpn_i++) {
		    alpn_str[alpn_str_len+alpn_i] = packet->payload[s_offset+alpn_i];
		  }

	        s_offset += alpn_len, alpn_str_len += alpn_len;;
	      } else {
	        ndpi_set_risk(ndpi_struct, flow, NDPI_TLS_UNCOMMON_ALPN, alpn_str);
	        break;
	      }
	    } else {
	      ndpi_set_risk(ndpi_struct, flow, NDPI_TLS_UNCOMMON_ALPN, alpn_str);
	      break;
	    }
	  }