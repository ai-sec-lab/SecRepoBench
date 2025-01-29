struct in_addr ip_addr;
      
      ip_addr.s_addr = inet_addr(name);
      if(strcmp(inet_ntoa(ip_addr), name) == 0)
	return(0);