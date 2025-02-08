static bfd *
alpha_ecoff_get_elt_at_filepos (bfd *archive, file_ptr filepos)
{
  bfd *nbfd = NULL;
  struct areltdata *tdata;
  struct ar_hdr *hdr;
  bfd_byte ab[8];
  bfd_size_type size;
  bfd_byte *buf, *p;
  struct bfd_in_memory *bim;
  ufile_ptr filesize;

  buf = NULL;
  nbfd = _bfd_get_elt_at_filepos (archive, filepos);
  if (nbfd == NULL)
    goto error_return;

  if ((nbfd->flags & BFD_IN_MEMORY) != 0)
    {
      /* We have already expanded this BFD.  */
      return nbfd;
    }

  tdata = (struct areltdata *) nbfd->arelt_data;
  hdr = (struct ar_hdr *) tdata->arch_header;
  if (strncmp (hdr->ar_fmag, ARFZMAG, 2) != 0)
    return nbfd;

  /* We must uncompress this element.  We do this by copying it into a
     memory buffer, and making bfd_bread and bfd_seek use that buffer.
     This can use a lot of memory, but it's simpler than getting a
     temporary file, making that work with the file descriptor caching
     code, and making sure that it is deleted at all appropriate
     times.  It can be changed if it ever becomes important.  */

  /* The compressed file starts with a dummy ECOFF file header.  */
  if (bfd_seek (nbfd, (file_ptr) FILHSZ, SEEK_SET) != 0)
    goto error_return;

  /* The next eight bytes are the real file size.  */
  if (bfd_bread (ab, (bfd_size_type) 8, nbfd) != 8)
    goto error_return;
  size = H_GET_64 (nbfd, ab);

  /* The decompression algorithm will at most expand by eight times.  */
  filesize = bfd_get_file_size (archive);
  if (filesize != 0 && size / 8 > filesize)
    {
      bfd_set_error (bfd_error_malformed_archive);
      goto error_return;
    }

  if (size != 0)
    {
      bfd_size_type remainingsize;
      bfd_byte dict[4096];
      unsigned int h;
      bfd_byte b;

      buf = (bfd_byte *) bfd_malloc (size);
      if (buf == NULL)
	goto error_return;
      p = buf;

      remainingsize = size;

      /* I don't know what the next eight bytes are for.  */
      if (bfd_bread (ab, (bfd_size_type) 8, nbfd) != 8)
	goto error_return;

      /* This is the uncompression algorithm.  It's a simple
	 dictionary based scheme in which each character is predicted
	 by a hash of the previous three characters.  A control byte
	 indicates whether the character is predicted or whether it
	 appears in the input stream; each control byte manages the
	 next eight bytes in the output stream.  */
      memset (dict, 0, sizeof dict);
      h = 0;
      while (bfd_bread (&b, (bfd_size_type) 1, nbfd) == 1)
	{
	  unsigned int i;

	  for (i = 0; i < 8; i++, b >>= 1)
	    {
	      bfd_byte n;

	      if ((b & 1) == 0)
		n = dict[h];
	      else
		{
		  if (bfd_bread (&n, 1, nbfd) != 1)
		    goto error_return;
		  dict[h] = n;
		}

	      *p++ = n;

	      --remainingsize;
	      if (remainingsize == 0)
		break;

	      h <<= 4;
	      h ^= n;
	      h &= sizeof dict - 1;
	    }

	  if (remainingsize == 0)
	    break;
	}
    }

  /* Now the uncompressed file contents are in buf.  */
  bim = ((struct bfd_in_memory *)
	 bfd_malloc ((bfd_size_type) sizeof (struct bfd_in_memory)));
  if (bim == NULL)
    goto error_return;
  bim->size = size;
  bim->buffer = buf;

  nbfd->mtime_set = TRUE;
  nbfd->mtime = strtol (hdr->ar_date, (char **) NULL, 10);

  nbfd->flags |= BFD_IN_MEMORY;
  nbfd->iostream = bim;
  nbfd->iovec = &_bfd_memory_iovec;
  nbfd->origin = 0;
  BFD_ASSERT (! nbfd->cacheable);

  return nbfd;

 error_return:
  if (buf != NULL)
    free (buf);
  if (nbfd != NULL)
    bfd_close (nbfd);
  return NULL;
}