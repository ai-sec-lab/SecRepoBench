static bool
copy_unknown_object (bfd *ibfd, bfd *outputbfd)
{
  char *cbuf;
  // <MASK>

  /* We should at least to be able to read it back when copying an
     unknown object in an archive.  */
  chmod (bfd_get_filename (outputbfd), buf.st_mode | S_IRUSR);
  free (cbuf);
  return true;
}