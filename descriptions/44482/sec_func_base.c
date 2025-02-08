const char * ndpi_strncasestr(const char *str1, const char *str2, size_t len) {
  size_t str1_len = strnlen(str1, len);
  size_t str2_len = strlen(str2);
  int i; /* signed! */

  for(i = 0; i < (int)(str1_len - str2_len + 1); i++){
    if(str1[0] == '\0')
      return NULL;
    else if(strncasecmp(str1, str2, str2_len) == 0)
      return(str1);

    str1++;
  }

  return NULL;
}