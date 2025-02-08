int
print_insn_ns32k (bfd_vma memaddr, disassemble_info *info)
{
  unsigned int opcodeindex;
  const char *d;
  unsigned short first_word;
  int ioffset;		/* Bits into instruction.  */
  int aoffset;		/* Bits into arguments.  */
  char arg_bufs[MAX_ARGS+1][ARG_LEN];
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
  for (opcodeindex = 0; opcodeindex < NOPCODES; opcodeindex++)
    if (ns32k_opcodes[opcodeindex].opcode_id_size <= 8
	&& ((buffer[0]
	     & (((unsigned long) 1 << ns32k_opcodes[opcodeindex].opcode_id_size) - 1))
	    == ns32k_opcodes[opcodeindex].opcode_seed))
      break;
  if (opcodeindex == NOPCODES)
    {
      /* Maybe it is 9 to 16 bits big.  */
      FETCH_DATA (info, buffer + 2);
      first_word = read_memory_integer(buffer, 2);

      for (opcodeindex = 0; opcodeindex < NOPCODES; opcodeindex++)
	if ((first_word
	     & (((unsigned long) 1 << ns32k_opcodes[opcodeindex].opcode_id_size) - 1))
	    == ns32k_opcodes[opcodeindex].opcode_seed)
	  break;

      /* Handle undefined instructions.  */
      if (opcodeindex == NOPCODES)
	{
	  (*dis_info->fprintf_func)(dis_info->stream, "0%o", buffer[0]);
	  return 1;
	}
    }

  (*dis_info->fprintf_func)(dis_info->stream, "%s", ns32k_opcodes[opcodeindex].name);

  ioffset = ns32k_opcodes[opcodeindex].opcode_size;
  aoffset = ns32k_opcodes[opcodeindex].opcode_size;
  d = ns32k_opcodes[opcodeindex].operands;

  if (*d)
    {
      /* Offset in bits of the first thing beyond each index byte.
	 Element 0 is for operand A and element 1 is for operand B.  */
      // <MASK>

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