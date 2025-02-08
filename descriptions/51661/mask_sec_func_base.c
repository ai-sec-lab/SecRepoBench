static bfd_cleanup
pdb_archive_p (bfd *abfd)
{
  int ret;
  char magic[sizeof (pdb_magic)];

  ret = bfd_bread (magic, sizeof (magic), abfd);
  if (ret != sizeof (magic))
    {
      bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  if (memcmp (magic, pdb_magic, sizeof (magic)))
    {
      bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  // <MASK>
}