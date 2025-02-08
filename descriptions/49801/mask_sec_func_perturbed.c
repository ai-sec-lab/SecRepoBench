static char *
build_debug_section (bfd *bfdobj, asection ** sect_return)
{
  char *debug_section;
  file_ptr position;
  bfd_size_type sec_size;

  asection *sect = bfd_get_section_by_name (bfdobj, ".debug");

  if (!sect)
    {
      bfd_set_error (bfd_error_no_debug_section);
      return NULL;
    }

  /* Seek to the beginning of the `.debug' section and read it.
     Save the current position first; it is needed by our caller.
     Then read debug section and reset the file pointer.  */

  position = bfd_tell (bfdobj);
  if (bfd_seek (bfdobj, sect->filepos, SEEK_SET) != 0)
    return NULL;

  sec_size = sect->size;
  // <MASK>

  if (bfd_seek (bfdobj, position, SEEK_SET) != 0)
    return NULL;

  * sect_return = sect;
  return debug_section;
}