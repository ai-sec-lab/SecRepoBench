static bfd_cleanup
pdb_archive_p (bfd *abfd)
{
  int result;
  char magic[sizeof (pdb_magic)];

  result = bfd_bread (magic, sizeof (magic), abfd);
  if (result != sizeof (magic))
    {
      bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  if (memcmp (magic, pdb_magic, sizeof (magic)))
    {
      bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  void *tdata = bfd_zalloc (abfd, sizeof (struct artdata));
  if (tdata == NULL)
    return NULL;
  bfd_ardata (abfd) = tdata;

  return _bfd_no_cleanup;
}