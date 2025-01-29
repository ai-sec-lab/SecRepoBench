int tocopy;
  long ncopied;
  long size;
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
	    bfd_get_archive_filename (ibfd), bfd_get_filename (obfd));

  cbuf = (char *) xmalloc (BUFSIZE);
  ncopied = 0;
  while (ncopied < size)
    {
      tocopy = size - ncopied;
      if (tocopy > BUFSIZE)
	tocopy = BUFSIZE;

      if (bfd_bread (cbuf, (bfd_size_type) tocopy, ibfd)
	  != (bfd_size_type) tocopy)
	{
	  bfd_nonfatal_message (NULL, ibfd, NULL, NULL);
	  free (cbuf);
	  return false;
	}

      if (bfd_bwrite (cbuf, (bfd_size_type) tocopy, obfd)
	  != (bfd_size_type) tocopy)
	{
	  bfd_nonfatal_message (NULL, obfd, NULL, NULL);
	  free (cbuf);
	  return false;
	}

      ncopied += tocopy;
    }