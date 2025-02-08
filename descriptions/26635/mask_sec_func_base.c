char *
_bfd_vms_save_counted_string (bfd *abfd, unsigned char *ptr, size_t maxlen)
{
  // <MASK>
  return _bfd_vms_save_sized_string (abfd, ptr, len);
}