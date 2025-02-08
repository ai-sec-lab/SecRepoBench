void macho_parse_file(
    const uint8_t* data,
    const uint64_t size,
    YR_OBJECT* object,
    YR_SCAN_CONTEXT* context)
{
  // Size must be large enough the hold yr_mach_header_64_t, which is larger
  // than yr_mach_header_32_t.
  if (size < sizeof(yr_mach_header_64_t))
    return;

  size_t header_size = macho_is_32(data) ? sizeof(yr_mach_header_32_t)
                                         : sizeof(yr_mach_header_64_t);

  // yr_mach_header_64_t is used for storing the header for both for 32-bits and
  // 64-bits files. yr_mach_header_64_t is exactly like yr_mach_header_32_t
  // but with an extra "reserved" field at the end.
  yr_mach_header_64_t header;

  memcpy(&header, data, header_size);

  int should_swap = should_swap_bytes(header.magic);

  if (should_swap)
    swap_mach_header(&header);

  set_integer(header.magic, object, "magic");
  set_integer(header.cputype, object, "cputype");
  set_integer(header.cpusubtype, object, "cpusubtype");
  set_integer(header.filetype, object, "filetype");
  set_integer(header.ncmds, object, "ncmds");
  set_integer(header.sizeofcmds, object, "sizeofcmds");
  set_integer(header.flags, object, "flags");

  // The "reserved" field exists only in 64 bits files.
  // <MASK>
}