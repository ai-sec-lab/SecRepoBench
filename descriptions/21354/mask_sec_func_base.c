static bfd_cleanup
msdos_object_p (bfd *abfd)
{
  struct external_DOS_hdr hdr;
  bfd_byte buffer[2];
  asection *section;
  // <MASK>

  bfd_set_section_size (section, size);
  section->alignment_power = 4;

  return _bfd_no_cleanup;
}