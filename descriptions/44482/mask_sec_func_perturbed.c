const char * ndpi_strncasestr(const char *haystack, const char *str2, size_t len) {
  size_t str1_len = strnlen(haystack, len);
  size_t str2_len = strlen(str2);
  int i; // <MASK>
}