void macho_parse_file(
    const uint8_t* data,
    const uint64_t filesize,
    YR_OBJECT* object,
    YR_SCAN_CONTEXT* context)
{
  // Size must be large enough the hold yr_mach_header_64_t, which is larger
  // than yr_mach_header_32_t.
  if (filesize < sizeof(yr_mach_header_64_t))
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
  if (!macho_is_32(data))
    set_integer(header.reserved, object, "reserved");

  // The first command parsing pass handles only segments.
  uint64_t seg_count = 0;
  uint64_t parsed_size = header_size;
  uint8_t* command = (uint8_t*) (data + header_size);

  yr_load_command_t command_struct;

  for (unsigned i = 0; i < header.ncmds; i++)
  {
    if (data + filesize < command + sizeof(yr_load_command_t))
      break;

    memcpy(&command_struct, command, sizeof(yr_load_command_t));

    if (should_swap)
      swap_load_command(&command_struct);

    if (filesize - parsed_size < command_struct.cmdsize)
      break;

    if (command_struct.cmdsize < sizeof(yr_load_command_t))
      break;

    switch (command_struct.cmd)
    {
    case LC_SEGMENT:
      if (command_struct.cmdsize >= sizeof(yr_segment_command_32_t))
        macho_handle_segment(command, seg_count++, object);
      break;
    case LC_SEGMENT_64:
      if (command_struct.cmdsize >= sizeof(yr_segment_command_64_t))
        macho_handle_segment_64(command, seg_count++, object);
      break;
    }

    command += command_struct.cmdsize;
    parsed_size += command_struct.cmdsize;
  }

  set_integer(seg_count, object, "number_of_segments");

  // The second command parsing pass handles others, who use segment count.
  parsed_size = header_size;
  command = (uint8_t*) (data + header_size);

  for (unsigned i = 0; i < header.ncmds; i++)
  {
    if (data + filesize < command + sizeof(yr_load_command_t))
      break;

    memcpy(&command_struct, command, sizeof(yr_load_command_t));

    if (should_swap)
      swap_load_command(&command_struct);

    if (filesize - parsed_size < command_struct.cmdsize)
      break;

    if (command_struct.cmdsize < sizeof(yr_load_command_t))
      break;

    switch (command_struct.cmd)
    {
    case LC_UNIXTHREAD:
      if (command_struct.cmdsize >= sizeof(yr_thread_command_t))
        macho_handle_unixthread(command, object, context);
      break;
    case LC_MAIN:
      if (command_struct.cmdsize >= sizeof(yr_entry_point_command_t))
        macho_handle_main(command, object, context);
      break;
    }

    command += command_struct.cmdsize;
    parsed_size += command_struct.cmdsize;
  }
}