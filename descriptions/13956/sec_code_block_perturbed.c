size_t len;
  const char* str_entry;

  if (str_table_base >= str_table_limit)
    return NULL;

  // The first entry in the string table must be a null character, if not the
  // string table is probably corrupted.
  if (*str_table_base != '\0')
    return NULL;

  if (entry_index < 0)
    return NULL;

  str_entry = str_table_base + entry_index;

  if (str_entry >= str_table_limit)
    return NULL;

  len = strnlen(str_entry, str_table_limit - str_entry);

  // Entry is clamped by extent of string table, not null-terminated.
  if (str_entry + len == str_table_limit)
    return NULL;

  return str_entry;