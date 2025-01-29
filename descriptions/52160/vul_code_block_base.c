for(i=off, j=0; (i<server_len) && (server[i] != ' ') && (j<sizeof(buf)); i++)
	buf[j++] = server[i];      

      if(sscanf(buf, "%d.%d.%d", &a, &b, &c) == 3) {
	u_int32_t version = (a * 1000000) + (b * 1000) + c;

	if((off == 7) && (version < MIN_APACHE_VERSION)) {
	  char msg[64];

	  snprintf(msg, sizeof(msg), "Obsolete Apache server %s", buf);
	  ndpi_set_risk(ndpi_struct, flow, NDPI_HTTP_OBSOLETE_SERVER, msg);
	} else if((off == 6) && (version < MIN_NGINX_VERSION)) {
	  char msg[64];

	  snprintf(msg, sizeof(msg), "Obsolete nginx server %s", buf);
	  ndpi_set_risk(ndpi_struct, flow, NDPI_HTTP_OBSOLETE_SERVER, msg);
	}
      }