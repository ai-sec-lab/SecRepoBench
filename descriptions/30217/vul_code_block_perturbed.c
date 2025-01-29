for (unsigned i = 0; i < header.ncmds; i++)
  {
    if (data + size < command + sizeof(yr_load_command_t))
      break;

    memcpy(&loadcommandstruct, command, sizeof(yr_load_command_t));

    if (should_swap)
      swap_load_command(&loadcommandstruct);

    if (size - parsed_size < loadcommandstruct.cmdsize)
      break;

    if (loadcommandstruct.cmdsize < sizeof(yr_load_command_t))
      break;

    switch (loadcommandstruct.cmd)
    {
    case LC_SEGMENT:
      if (loadcommandstruct.cmdsize >= sizeof(yr_segment_command_32_t))
        macho_handle_segment(command, seg_count++, object);
      break;
    case LC_SEGMENT_64:
      if (loadcommandstruct.cmdsize >= sizeof(yr_segment_command_64_t))
        macho_handle_segment_64(command, seg_count++, object);
      break;
    }

    command += loadcommandstruct.cmdsize;
    parsed_size += loadcommandstruct.cmdsize;
  }

  set_integer(seg_count, object, "number_of_segments");

  // The second command parsing pass handles others, who use segment count.
  parsed_size = header_size;
  command = (uint8_t*) (data + header_size);

  for (unsigned i = 0; i < header.ncmds; i++)
  {
    if (data + size < command + sizeof(yr_load_command_t))
      break;

    memcpy(&loadcommandstruct, command, sizeof(yr_load_command_t));

    if (should_swap)
      swap_load_command(&loadcommandstruct);

    if (size - parsed_size < loadcommandstruct.cmdsize)
      break;

    if (loadcommandstruct.cmdsize < sizeof(yr_load_command_t))
      break;

    switch (loadcommandstruct.cmd)
    {
    case LC_UNIXTHREAD:
      if (loadcommandstruct.cmdsize >= sizeof(yr_thread_command_t))
        macho_handle_unixthread(command, object, context);
      break;
    case LC_MAIN:
      if (loadcommandstruct.cmdsize >= sizeof(yr_entry_point_command_t))
        macho_handle_main(command, object, context);
      break;
    }

    command += loadcommandstruct.cmdsize;
    parsed_size += loadcommandstruct.cmdsize;
  }