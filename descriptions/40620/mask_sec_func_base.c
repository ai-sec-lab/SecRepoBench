bool
bfd_get_full_section_contents (bfd *abfd, sec_ptr sec, bfd_byte **ptr)
{
  bfd_size_type sz;
  bfd_byte *p = *ptr;
  bool ret;
  bfd_size_type save_size;
  bfd_size_type save_rawsize;
  bfd_byte *compressed_buffer;
  unsigned int compression_header_size;
  // <MASK>
}