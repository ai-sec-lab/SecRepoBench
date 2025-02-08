static bfd_boolean
vms_traverse_index (bfd *abfd, unsigned int vbn, struct carsym_mem *cs,
		    unsigned int recur_count)
{
  struct vms_indexdef indexdef;
  file_ptr off;
  unsigned char *p;
  unsigned char *endp;
  unsigned int n;

  if (recur_count == 100)
    {
      bfd_set_error (bfd_error_bad_value);
      return FALSE;
    }

  /* Read the index block.  */
  BFD_ASSERT (sizeof (indexdef) == VMS_BLOCK_SIZE);
  if (!vms_read_block (abfd, vbn, &indexdef))
    return FALSE;

  /* Traverse it.  */
  p = &indexdef.keys[0];
  n = bfd_getl16 (indexdef.used);
  if (n > sizeof (indexdef.keys))
    return FALSE;
  endp = p + n;
  while (p < endp)
    {
      unsigned int indexvbn;
      unsigned int idx_off;
      unsigned int keylen;
      unsigned char *keyname;
      unsigned int flags;

      /* Extract key length.  */
      // <MASK>
    }

  return TRUE;
}