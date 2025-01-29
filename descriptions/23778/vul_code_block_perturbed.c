
  if (bfd_seek (abfd, where, SEEK_SET) != 0)
    return NULL;

  if (bfd_bread (buffer, 256, abfd) < 4)
    return NULL;

  /* Ensure null termination of filename.  */
  buffer[256] = '\0';