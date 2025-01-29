debug_section = (char *) _bfd_alloc_and_read (abfd, sec_size, sec_size);
  if (debug_section == NULL)
    return NULL;