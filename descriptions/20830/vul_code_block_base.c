/* xxxx LOGIN "username" "password" */
	  char str[256], *item;
	  u_int len = packet->payload_packet_len > sizeof(str) ? sizeof(str) : packet->payload_packet_len;
	  
	  strncpy(str, (const char*)packet->payload, len);
	  str[len] = '\0';

	  item = strchr(str, '"');
	  if(item) {
	    char *column;
	    
	    item++;
	    column = strchr(item, '"');

	    if(column) {
	      column[0] = '\0';
	      snprintf(flow->protos.ftp_imap_pop_smtp.username,
		       sizeof(flow->protos.ftp_imap_pop_smtp.username),
		       "%s", item);

	      column = strchr(&column[1], '"');
	      if(column) {
		item = &column[1];
		column = strchr(item, '"');

		if(column) {
		  column[0] = '\0';
		  snprintf(flow->protos.ftp_imap_pop_smtp.password,
			   sizeof(flow->protos.ftp_imap_pop_smtp.password),
			   "%s", item);
		}
	      }
	    }
	  }
	  
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;