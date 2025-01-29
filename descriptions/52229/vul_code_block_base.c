u_int i, j, a, b, c;
      char buf[16] = { '\0' };

      for(i=off, j=0; (i<server_len) && (j<sizeof(buf))
	    && (isdigit(server[i]) || (server[i] == '.')); i++)
	buf[j++] = server[i];      

      if(sscanf(buf, "%d.%d.%d", &a, &b, &c) == 3) {
	u_int32_t version = (a * 1000000) + (b * 1000) + c;
	char msg[64];
	
	if((off == 7) && (version < MIN_APACHE_VERSION)) {
	  snprintf(msg, sizeof(msg), "Obsolete Apache server %s", buf);
	  ndpi_set_risk(ndpi_struct, flow, NDPI_HTTP_OBSOLETE_SERVER, msg);
	} else if((off == 6) && (version < MIN_NGINX_VERSION)) {
	  snprintf(msg, sizeof(msg), "Obsolete nginx server %s", buf);
	  ndpi_set_risk(ndpi_struct, flow, NDPI_HTTP_OBSOLETE_SERVER, msg);
	}
      }