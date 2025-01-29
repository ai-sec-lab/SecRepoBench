unsigned int size;
  enum map_type mapping_type = MAP_CODE;

  if (info->private_data == NULL)
    {
      /* Note: remain lifecycle throughout whole execution.  */
      static struct nds32_private_data private;
      private.has_mapping_symbols = -1;	/* unknown yet.  */
      private.last_symbol_index = -1;
      private.last_addr = 0;
      info->private_data = &private;
    }
  private_data = info->private_data;

  if (info->symtab_size != 0)
    {
      int start;
      if (pc == 0)
	start = 0;
      else
	{
	  start = info->symtab_pos;
	  if (start < private_data->last_symbol_index)
	    start = private_data->last_symbol_index;
	}

      if (0 > start)
	start = 0;

      if (private_data->has_mapping_symbols != 0
	  && ((strncmp (".text", info->section->name, 5) == 0)))
	{
	  for (symbolindex = start; symbolindex < info->symtab_size; symbolindex++)
	    {
	      addr = bfd_asymbol_value (info->symtab[symbolindex]);
	      if (addr > pc)
		break;
	      if (get_mapping_symbol_type (info, symbolindex, &mapping_type))
		{
		  last_symbol_index = symbolindex;
		  found = TRUE;
		}
	    }

	  if (found)
	    private_data->has_mapping_symbols = 1;
	  else if (!found && private_data->has_mapping_symbols == -1)
	    {
	      /* Make sure there are no any mapping symbol.  */
	      for (symbolindex = 0; symbolindex < info->symtab_size; symbolindex++)
		{
		  if (is_mapping_symbol (info, symbolindex, &mapping_type))
		    {
		      private_data->has_mapping_symbols = -1;
		      break;
		    }
		}
	      if (private_data->has_mapping_symbols == -1)
		private_data->has_mapping_symbols = 0;
	    }

	  private_data->last_symbol_index = last_symbol_index;
	  private_data->last_mapping_type = mapping_type;
	  is_data = (private_data->last_mapping_type == MAP_DATA0
		     || private_data->last_mapping_type == MAP_DATA1
		     || private_data->last_mapping_type == MAP_DATA2
		     || private_data->last_mapping_type == MAP_DATA3
		     || private_data->last_mapping_type == MAP_DATA4);
	}
    }

  /* Wonder data or instruction.  */
  if (is_data)
    {
      unsigned int i1;

      /* Fix corner case: there is no next mapping symbol,
	 let mapping type decides size */
      size = 16;
      if (last_symbol_index + 1 >= info->symtab_size)
	{
	  if (mapping_type == MAP_DATA0)
	    size = 1;
	  if (mapping_type == MAP_DATA1)
	    size = 2;
	  if (mapping_type == MAP_DATA2)
	    size = 4;
	  if (mapping_type == MAP_DATA3)
	    size = 8;
	  if (mapping_type == MAP_DATA4)
	    size = 16;
	}
      for (symbolindex = last_symbol_index + 1; symbolindex < info->symtab_size; symbolindex++)
	{
	  addr = bfd_asymbol_value (info->symtab[symbolindex]);

	  enum map_type fake_mapping_type;
	  if (get_mapping_symbol_type (info, symbolindex, &fake_mapping_type)
	      && (addr > pc
		  && ((info->section == NULL)
		      || (info->section == info->symtab[symbolindex]->section)))
	      && (addr - pc < size))
	    {
	      size = addr - pc;
	      break;
	    }
	}

      if (size == 3)
	size = (pc & 1) ? 1 : 2;

      /* Read bytes from BFD.  */
      info->read_memory_func (pc, buf_data, size, info);
      given = 0;
      given1 = 0;
      /* Start assembling data.  */
      /* Little endian of data.  */
      if (info->endian == BFD_ENDIAN_LITTLE)
	{
	  for (i1 = size - 1;; i1--)
	    {
	      if (i1 >= 8)
		given1 = buf_data[i1] | (given1 << 8);
	      else
		given = buf_data[i1] | (given << 8);

	      if (i1 == 0)
		break;
	    }
	}
      else
	{
	  /* Big endian of data.  */
	  for (i1 = 0; i1 < size; i1++)
	    {
	      if (i1 <= 7)
		given = buf_data[i1] | (given << 8);
	      else
		given1 = buf_data[i1] | (given1 << 8);
	    }
	}

      info->bytes_per_line = 4;

      if (size == 16)
	info->fprintf_func (info->stream, ".qword\t0x%016" PRIx64 "%016" PRIx64,
			    given, given1);
      else if (size == 8)
	info->fprintf_func (info->stream, ".dword\t0x%016" PRIx64, given);
      else if (size == 4)
	info->fprintf_func (info->stream, ".word\t0x%08" PRIx64, given);
      else if (size == 2)
	{
	  /* short */
	  if (mapping_type == MAP_DATA0)
	    info->fprintf_func (info->stream, ".byte\t0x%02" PRIx64,
				given & 0xFF);
	  else
	    info->fprintf_func (info->stream, ".short\t0x%04" PRIx64, given);
	}
      else
	{
	  /* byte */
	  info->fprintf_func (info->stream, ".byte\t0x%02" PRIx64, given);
	}

      return size;
    }

  size = 4;
  status = info->read_memory_func (pc, buf, 4, info);
  if (status)
    {
      /* For the last 16-bit instruction.  */
      size = 2;
      status = info->read_memory_func (pc, buf, 2, info);
      if (status)
	{
	  (*info->memory_error_func) (status, pc, info);
	  return -1;
	}
      buf[2] = 0;
      buf[3] = 0;
    }

  insn = bfd_getb32 (buf);
  /* 16-bit instruction.  */
  if (insn & 0x80000000)
    {
      print_insn16 (pc, info, (insn >> 16), NDS32_PARSE_INSN16);
      return 2;
    }

  /* 32-bit instructions.  */
  if (size == 4)
    print_insn32 (pc, info, insn, NDS32_PARSE_INSN32);
  else
    info->fprintf_func (info->stream,
			_("insufficient data to decode instruction"));
  return 4;