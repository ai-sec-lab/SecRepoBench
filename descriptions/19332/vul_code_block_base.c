while((offset+2) < payload_length) {
        u_int16_t attribute = ntohs(*((u_int16_t*)&payload[offset]));
        u_int16_t len = ntohs(*((u_int16_t*)&payload[offset+2]));
        u_int16_t x = (len + 4) % 4;

        if(x)
          len += 4-x;

#ifdef DEBUG_STUN
        printf("==> Attribute: %04X\n", attribute);
#endif

        switch(attribute) {
	case 0x0103:
          flow->guessed_host_protocol_id = NDPI_PROTOCOL_ZOOM;
          return(NDPI_IS_STUN);
	  break;
	  
        case 0x4000:
        case 0x4001:
        case 0x4002:
          /* These are the only messages apparently whatsapp voice can use */
          flow->guessed_host_protocol_id = NDPI_PROTOCOL_WHATSAPP_CALL;
          return(NDPI_IS_STUN);
          break;

        case 0x0014: /* Realm */
        {
          u_int16_t realm_len = ntohs(*((u_int16_t*)&payload[offset+2]));

          if(flow->host_server_name[0] == '\0') {
            u_int j, i = (realm_len > sizeof(flow->host_server_name)) ? sizeof(flow->host_server_name) : realm_len;
            u_int k = offset+4;

            memset(flow->host_server_name, 0, sizeof(flow->host_server_name));

            for(j=0; j<i; j++) {
	      if((k+i) < payload_length)
		flow->host_server_name[j] = payload[k++];
	      else
		break;
	    }
	    
#ifdef DEBUG_STUN
            printf("==> [%s]\n", flow->host_server_name);
#endif

            if(strstr((char*) flow->host_server_name, "google.com") != NULL) {
                flow->guessed_host_protocol_id = NDPI_PROTOCOL_HANGOUT_DUO;
                return(NDPI_IS_STUN);
            } else if(strstr((char*) flow->host_server_name, "whispersystems.org") != NULL) {
              flow->guessed_host_protocol_id = NDPI_PROTOCOL_SIGNAL;
              return(NDPI_IS_STUN);
            }
          }
        }
        break;

        case 0xC057: /* Messeger */
          if(msg_type == 0x0001) {
            if((msg_len == 100) || (msg_len == 104)) {
              flow->guessed_host_protocol_id = NDPI_PROTOCOL_MESSENGER;
              return(NDPI_IS_STUN);
            } else if(msg_len == 76) {
#if 0
              if(1) {
                flow->guessed_host_protocol_id = NDPI_PROTOCOL_HANGOUT_DUO;
                return(NDPI_IS_NOT_STUN); /* This case is found also with signal traffic */
              } else
                return(NDPI_IS_STUN);
#endif
            }
          }
          break;

        case 0x8054: /* Candidate Identifier */
          if((len == 4)
             && ((offset+7) < payload_length)
             && (payload[offset+5] == 0x00)
             && (payload[offset+6] == 0x00)
             && (payload[offset+7] == 0x00)) {
            /* Either skype for business or "normal" skype with multiparty call */
#ifdef DEBUG_STUN
            printf("==> Skype found\n");
#endif
            flow->guessed_host_protocol_id = NDPI_PROTOCOL_SKYPE_CALL;
            return(NDPI_IS_STUN);
          }

          break;

        case 0x8055: /* MS Service Quality (skype?) */
          break;

          /* Proprietary fields found on skype calls */
        case 0x24DF:
        case 0x3802:
        case 0x8036:
        case 0x8095:
        case 0x0800:
        case 0x8006: /* This is found on skype calls) */
          /* printf("====>>>> %04X\n", attribute); */
#ifdef DEBUG_STUN
          printf("==> Skype (2) found\n");
#endif

          flow->guessed_host_protocol_id = NDPI_PROTOCOL_SKYPE_CALL;
          return(NDPI_IS_STUN);
          break;

        case 0x8070: /* Implementation Version */
          if(len == 4 && ((offset+7) < payload_length)
             && (payload[offset+4] == 0x00) && (payload[offset+5] == 0x00) && (payload[offset+6] == 0x00) &&
             ((payload[offset+7] == 0x02) || (payload[offset+7] == 0x03))) {
#ifdef DEBUG_STUN
            printf("==> Skype (3) found\n");
#endif

            flow->guessed_host_protocol_id = NDPI_PROTOCOL_SKYPE_CALL;
            return(NDPI_IS_STUN);
          }
          break;

        case 0xFF03:
          flow->guessed_host_protocol_id = NDPI_PROTOCOL_HANGOUT_DUO;
          return(NDPI_IS_STUN);
          break;

        default:
#ifdef DEBUG_STUN
          printf("==> %04X\n", attribute);
#endif
          break;
        }

        offset += len + 4;
      }