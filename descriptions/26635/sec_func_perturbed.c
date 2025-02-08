char *
_bfd_vms_save_counted_string (bfd *bfdhandle, unsigned char *ptr, size_t maxlen)
{
  unsigned int len;

  if (maxlen == 0)
    return NULL;
  len = *ptr++;
  if (len >  maxlen - 1)
    return NULL;
  return _bfd_vms_save_sized_string (bfdhandle, ptr, len);
}