size_t len;
  const char* str_entry = str_table_base + index;

  // The first entry in the string table must be a null character, if not the
  // string table is probably corrupted.
  if (*str_table_base != '\0')
    return NULL;

  if (index < 0)
    return NULL;
