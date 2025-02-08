char *
_bfd_vms_save_counted_string (bfd *bfdhandle, unsigned char *ptr, size_t maxlen)
{
  // <MASK>
  return _bfd_vms_save_sized_string (bfdhandle, ptr, len);
}