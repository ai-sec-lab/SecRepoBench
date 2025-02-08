  unsigned int chars_on_line;
  bfd_boolean is_eof = FALSE;

  /* To the front of the file.  */
  if (bfd_seek (abfd, (file_ptr) 0, SEEK_SET) != 0)
    return FALSE;

  while (! is_eof)
    {
      char src[MAXCHUNK];
      char type;

      /* Find first '%'.  */
      is_eof = (bfd_boolean) (bfd_bread (src, (bfd_size_type) 1, abfd) != 1);
      while (*src != '%' && !is_eof)
	is_eof = (bfd_boolean) (bfd_bread (src, (bfd_size_type) 1, abfd) != 1);

      if (is_eof)
	break;

      /* Fetch the type and the length and the checksum.  */
      if (bfd_bread (src, (bfd_size_type) 5, abfd) != 5)
	return FALSE;

      type = src[2];

      if (!ISHEX (src[0]) || !ISHEX (src[1]))
	break;

      /* Already read five chars.  */
      chars_on_line = HEX (src) - 5;

      if (chars_on_line >= MAXCHUNK)
	return FALSE;

      if (bfd_bread (src, (bfd_size_type) chars_on_line, abfd) != chars_on_line)
	return FALSE;

      /* Put a null at the end.  */
      src[chars_on_line] = 0;
      if (!func (abfd, type, src, src + chars_on_line))
	return FALSE;
    }

  return TRUE;