static bfd_cleanup
msdos_object_p (bfd *bfdfile)
{
  struct external_DOS_hdr hdr;
  bfd_byte buffer[2];
  asection *section;
  bfd_size_type size;

  if (bfd_seek (bfdfile, (file_ptr) 0, SEEK_SET) != 0
      || (size = bfd_bread (&hdr, sizeof (hdr), bfdfile)) < DOS_HDR_SIZE)
    {
      if (bfd_get_error () != bfd_error_system_call)
	bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  if (H_GET_16 (bfdfile, hdr.e_magic) != IMAGE_DOS_SIGNATURE)
    {
      bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  /* Check that this isn't actually a PE, NE, or LE file. If it is, the
     e_lfanew field will be valid and point to a header beginning with one of
     the relevant signatures.  If not, e_lfanew might point to anything, so
     don't bail if we can't read there.  */
  if (size < offsetof (struct external_DOS_hdr, e_lfanew) + 4
      || H_GET_16 (bfdfile, hdr.e_cparhdr) < 4)
    ;
  else if (bfd_seek (bfdfile, H_GET_32 (bfdfile, hdr.e_lfanew), SEEK_SET) != 0
	   || bfd_bread (buffer, (bfd_size_type) 2, bfdfile) != 2)
    {
      if (bfd_get_error () == bfd_error_system_call)
	return NULL;
    }
  else
    {
      if (H_GET_16 (bfdfile, buffer) == IMAGE_NT_SIGNATURE
	  || H_GET_16 (bfdfile, buffer) == IMAGE_OS2_SIGNATURE
	  || H_GET_16 (bfdfile, buffer) == IMAGE_OS2_SIGNATURE_LE
	  || H_GET_16 (bfdfile, buffer) == IMAGE_OS2_SIGNATURE_LX)
	{
	  bfd_set_error (bfd_error_wrong_format);
	  return NULL;
	}
    }

  if (!msdos_mkobject (bfdfile))
    return NULL;

  bfdfile->flags = EXEC_P;
  bfdfile->start_address = H_GET_16 (bfdfile, hdr.e_ip);

  section = bfd_make_section (bfdfile, ".text");
  if (section == NULL)
    return NULL;

  section->flags = (SEC_ALLOC | SEC_LOAD | SEC_CODE | SEC_HAS_CONTENTS);
  section->filepos = H_GET_16 (bfdfile, hdr.e_cparhdr) * 16;
  size = (H_GET_16 (bfdfile, hdr.e_cp) - 1) * EXE_PAGE_SIZE - section->filepos;
  size += H_GET_16 (bfdfile, hdr.e_cblp);

  /* Check that the size is valid.  */
  if (bfd_seek (bfdfile, section->filepos + size, SEEK_SET) != 0)
    {
      if (bfd_get_error () != bfd_error_system_call)
	bfd_set_error (bfd_error_wrong_format);
      return NULL;
    }

  bfd_set_section_size (section, size);
  section->alignment_power = 4;

  return _bfd_no_cleanup;
}