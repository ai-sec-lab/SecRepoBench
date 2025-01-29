s_offset += 2;
		tot_alpn_len += s_offset;

		while(s_offset < tot_alpn_len) {
		  u_int8_t alpn_i, alpn_len = packet->payload[s_offset++];

		  if((s_offset + alpn_len) <= tot_alpn_len) {
#ifdef DEBUG_TLS
		    printf("Client SSL [ALPN: %u]\n", alpn_len);
#endif

		    if((alpn_str_len+alpn_len+1) < sizeof(alpn_str)) {
		      if(alpn_str_len > 0) {
			alpn_str[alpn_str_len] = ',';
			alpn_str_len++;
		      }

		      for(alpn_i=0; alpn_i<alpn_len; alpn_i++)
			alpn_str[alpn_str_len+alpn_i] =  packet->payload[s_offset+alpn_i];

		      s_offset += alpn_len, alpn_str_len += alpn_len;;
		    } else
		      break;
		  } else
		    break;
		}