static bool
copy_unknown_object (bfd *ibfd, bfd *outputbfd)
{
  char *cbuf;
  bfd_size_type tocopy;
  off_t size;
  struct stat buf;

  if (bfd_stat_arch_elt (ibfd, &buf) != 0)
    {
      bfd_nonfatal_message (NULL, ibfd, NULL, NULL);
      return false;
    }

  size = buf.st_size;
  if (size < 0)
    {
      non_fatal (_("stat returns negative size for `%s'"),
		 bfd_get_archive_filename (ibfd));
      return false;
    }

  if (bfd_seek (ibfd, (file_ptr) 0, SEEK_SET) != 0)
    {
      bfd_nonfatal (bfd_get_archive_filename (ibfd));
      return false;
    }

  if (verbose)
    printf (_("copy from `%s' [unknown] to `%s' [unknown]\n"),
	    bfd_get_archive_filename (ibfd), bfd_get_filename (outputbfd));

  cbuf = (char *) xmalloc (BUFSIZE);
  while (size != 0)
    {
      if (size > BUFSIZE)
	tocopy = BUFSIZE;
      else
	tocopy = size;

      if (bfd_bread (cbuf, tocopy, ibfd) != tocopy)
	{
	  bfd_nonfatal_message (NULL, ibfd, NULL, NULL);
	  free (cbuf);
	  return false;
	}

      if (bfd_bwrite (cbuf, tocopy, outputbfd) != tocopy)
	{
	  bfd_nonfatal_message (NULL, outputbfd, NULL, NULL);
	  free (cbuf);
	  return false;
	}

      size -= tocopy;
    }

  /* We should at least to be able to read it back when copying an
     unknown object in an archive.  */
  chmod (bfd_get_filename (outputbfd), buf.st_mode | S_IRUSR);
  free (cbuf);
  return true;
}