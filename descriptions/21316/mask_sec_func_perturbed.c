bfd_boolean
_bfd_xcoff_slurp_armap (bfd *abfd)
{
  file_ptr off;
  size_t namlen;
  bfd_size_type sz;
  bfd_byte *contents, *cend;
  bfd_vma c, index;
  carsym *arsym;
  bfd_byte *p;

  if (xcoff_ardata (abfd) == NULL)
    {
      abfd->has_armap = FALSE;
      return TRUE;
    }

  if (! xcoff_big_format_p (abfd))
    // <MASK>

  /* After the file offsets come null terminated symbol names.  */
  cend = contents + sz;
  for (index = 0, arsym = bfd_ardata (abfd)->symdefs;
       index < c;
       ++index, ++arsym, p += strlen ((char *) p) + 1)
    {
      if (p >= cend)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return FALSE;
	}
      arsym->name = (char *) p;
    }

  bfd_ardata (abfd)->symdef_count = c;
  abfd->has_armap = TRUE;

  return TRUE;
}