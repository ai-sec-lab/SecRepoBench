debug_section = (char *) _bfd_alloc_and_read (abfd, sec_size + 1, sec_size);
  if (debug_section == NULL)
    return NULL;
  debug_section[sec_size] = 0;