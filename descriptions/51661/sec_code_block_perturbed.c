void *tdata = bfd_zalloc (abfd, sizeof (struct artdata));
  if (tdata == NULL)
    return NULL;
  bfd_ardata (abfd) = tdata;

  return _bfd_no_cleanup;