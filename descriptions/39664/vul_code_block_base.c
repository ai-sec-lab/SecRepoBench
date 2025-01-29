struct in_addr ip_addr;
      char buf[22];
      
      ip_addr.s_addr = inet_addr(buf);
      if(strcmp(inet_ntoa(ip_addr), buf) == 0)
	return(0);