/* signed! */

  for(i = 0; i < (int)(str1_len - str2_len + 1); i++){
    if(haystack[0] == '\0')
      return NULL;
    else if(strncasecmp(haystack, str2, str2_len) == 0)
      return(haystack);

    haystack++;
  }

  return NULL;