      u_int i, max_len;

      value = &attr[5];
      if(value[0] == '"')
	value++; /* remove leading " */

      max_len = strlen(value) - 1;
      if(value[max_len] == '"')
	value[max_len] = '\0'; /* remove trailing " */

      for(i=0; i<max_len; i++) value[i] = tolower(value[i]);