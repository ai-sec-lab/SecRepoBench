int
print_insn_ns32k (bfd_vma memaddr, disassemble_info *info)
{
  unsigned int i;
  const char *d;
  unsigned short first_word;
  int ioffset;		/* Bits into instruction.  */
  int aoffset;		/* Bits into arguments.  */
  /* The arg_bufs array is made static in order to avoid a potential
     use of an uninitialised value if we are asekd to disassemble a
     corrupt instruction.  */
  static char arg_bufs[MAX_ARGS+1][ARG_LEN];
  int argnum;
  int maxarg;
  struct private priv;
  bfd_byte *buffer = priv.the_buffer;
  dis_info = info;

  info->private_data = & priv;
  priv.max_fetched = priv.the_buffer;
  priv.insn_start = memaddr;
  if (OPCODES_SIGSETJMP (priv.bailout) != 0)
    /* Error return.  */
    return -1;

  /* Look for 8bit opcodes first. Other wise, fetching two bytes could take
     us over the end of accessible data unnecessarilly.  */
  FETCH_DATA (info, buffer + 1);
  for (i = 0; i < NOPCODES; i++)
    if (ns32k_opcodes[i].opcode_id_size <= 8
	&& ((buffer[0]
	     & (((unsigned long) 1 << ns32k_opcodes[i].opcode_id_size) - 1))
	    == ns32k_opcodes[i].opcode_seed))
      break;
  if (i == NOPCODES)
    {
      /* Maybe it is 9 to 16 bits big.  */
      FETCH_DATA (info, buffer + 2);
      first_word = read_memory_integer(buffer, 2);

      for (i = 0; i < NOPCODES; i++)
	if ((first_word
	     & (((unsigned long) 1 << ns32k_opcodes[i].opcode_id_size) - 1))
	    == ns32k_opcodes[i].opcode_seed)
	  break;

      /* Handle undefined instructions.  */
      if (i == NOPCODES)
	{
	  (*dis_info->fprintf_func)(dis_info->stream, "0%o", buffer[0]);
	  return 1;
	}
    }

  (*dis_info->fprintf_func)(dis_info->stream, "%s", ns32k_opcodes[i].name);

  ioffset = ns32k_opcodes[i].opcode_size;
  aoffset = ns32k_opcodes[i].opcode_size;
  d = ns32k_opcodes[i].operands;

  if (*d)
    {
      /* Offset in bits of the first thing beyond each index byte.
	 Element 0 is for operand A and element 1 is for operand B.  */
      int index_offset[2];

      /* 0 for operand A, 1 for operand B, greater for other args.  */
      int whicharg = 0;

      (*dis_info->fprintf_func)(dis_info->stream, "\t");

      maxarg = 0;

      /* First we have to find and keep track of the index bytes,
	 if we are using scaled indexed addressing mode, since the index
	 bytes occur right after the basic instruction, not as part
	 of the addressing extension.  */
      index_offset[0] = -1;
      index_offset[1] = -1;
      if (Is_gen (d[1]))
	{
	  int bitoff = d[1] == 'f' ? 10 : 5;
	  int addr_mode = bit_extract (buffer, ioffset - bitoff, 5);

	  if (Adrmod_is_index (addr_mode))
	    {
	      aoffset += 8;
	      index_offset[0] = aoffset;
	    }
	}

      if (d[2] && Is_gen (d[3]))
	{
	  int addr_mode = bit_extract (buffer, ioffset - 10, 5);

	  if (Adrmod_is_index (addr_mode))
	    {
	      aoffset += 8;
	      index_offset[1] = aoffset;
	    }
	}

      while (*d)
	{
	  argnum = *d - '1';
	  if (argnum >= MAX_ARGS)
	    abort ();
	  d++;
	  if (argnum > maxarg)
	    maxarg = argnum;
	  ioffset = print_insn_arg (*d, ioffset, &aoffset, buffer,
				    memaddr, arg_bufs[argnum],
				    whicharg > 1 ? -1 : index_offset[whicharg]);
	  d++;
	  whicharg++;
	}

      for (argnum = 0; argnum <= maxarg; argnum++)
	{
	  bfd_vma addr;
	  char *ch;

	  for (ch = arg_bufs[argnum]; *ch;)
	    {
	      if (*ch == NEXT_IS_ADDR)
		{
		  ++ch;
		  addr = bfd_scan_vma (ch, NULL, 16);
		  (*dis_info->print_address_func) (addr, dis_info);
		  while (*ch && *ch != NEXT_IS_ADDR)
		    ++ch;
		  if (*ch)
		    ++ch;
		}
	      else
		(*dis_info->fprintf_func)(dis_info->stream, "%c", *ch++);
	    }
	  if (argnum < maxarg)
	    (*dis_info->fprintf_func)(dis_info->stream, ", ");
	}
    }
  return aoffset / 8;
}