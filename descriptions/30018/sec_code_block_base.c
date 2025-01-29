if (!macho_is_32(data))
    set_integer(header.reserved, object, "reserved");

  // The first command parsing pass handles only segments.
  uint64_t seg_count = 0;
  uint64_t parsed_size = header_size;
  uint8_t* command = (uint8_t*) (data + header_size);

  yr_load_command_t command_struct;

  for (unsigned i = 0; i < header.ncmds; i++)
  {
    if (data + size < command + sizeof(yr_load_command_t))
      break;

    memcpy(&command_struct, command, sizeof(yr_load_command_t));

    if (should_swap)
      swap_load_command(&command_struct);

    if (size - parsed_size < command_struct.cmdsize)
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
    if (data + size < command + sizeof(yr_load_command_t))
      break;

    memcpy(&command_struct, command, sizeof(yr_load_command_t));

    if (should_swap)
      swap_load_command(&command_struct);

    if (size - parsed_size < command_struct.cmdsize)
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