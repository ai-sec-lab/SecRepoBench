const char * ndpi_strncasestr(const char *haystack, const char *str2, size_t len) {
  size_t str1_len = strnlen(haystack, len);
  size_t str2_len = strlen(str2);
  int i; /* signed! */

  for(i = 0; i < (int)(str1_len - str2_len + 1); i++){
    if(haystack[0] == '\0')
      return NULL;
    else if(strncasecmp(haystack, str2, str2_len) == 0)
      return(haystack);

    haystack++;
  }

  return NULL;
}