if (blob != (const void *) NULL)
    (void) memcpy(string_info->datum,blob,size);
  else
    (void) memset(string_info->datum,0,size);