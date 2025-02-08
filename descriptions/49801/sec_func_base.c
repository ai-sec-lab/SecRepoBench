static char *
build_debug_section (bfd *abfd, asection ** sect_return)
{
  char *debug_section;
  file_ptr position;
  bfd_size_type sec_size;

  asection *sect = bfd_get_section_by_name (abfd, ".debug");

  if (!sect)
    {
      bfd_set_error (bfd_error_no_debug_section);
      return NULL;
    }

  /* Seek to the beginning of the `.debug' section and read it.
     Save the current position first; it is needed by our caller.
     Then read debug section and reset the file pointer.  */

  position = bfd_tell (abfd);
  if (bfd_seek (abfd, sect->filepos, SEEK_SET) != 0)
    return NULL;

  sec_size = sect->size;
  debug_section = (char *) _bfd_alloc_and_read (abfd, sec_size + 1, sec_size);
  if (debug_section == NULL)
    return NULL;
  debug_section[sec_size] = 0;

  if (bfd_seek (abfd, position, SEEK_SET) != 0)
    return NULL;

  * sect_return = sect;
  return debug_section;
}