static bool
print_insn_cde (struct disassemble_info *info, long given, bool thumb)
{
  const struct cdeopcode32 *insn;
  void *stream = info->stream;
  fprintf_ftype func = info->fprintf_func;

  if (thumb)
  {
    /* Manually extract the coprocessor code from a known point.
       This position is the same across all CDE instructions.  */
    for (insn = cde_opcodes; insn->assembler; insn++)
    {
      uint16_t coproc = (given >> insn->coproc_shift) & insn->coproc_mask;
      uint16_t coproc_mask = 1 << coproc;
      if (! (coproc_mask & cde_coprocs))
	continue;

      if ((given & insn->mask) == insn->value)
      {
	bool is_unpredictable = false;
	const char *c;

	for (c = insn->assembler; *c; c++)
	{
	  if (*c == '%')
	  {
	    switch (*++c)
	    {
	      case '%':
		func (stream, "%%");
		break;

	      case '0': case '1': case '2': case '3': case '4':
	      case '5': case '6': case '7': case '8': case '9':
	      {
		int width;
		unsigned long value;

		c = arm_decode_bitfield (c, given, &value, &width);

		switch (*c)
		{
		  case 'S':
		    if (value > 10)
		      is_unpredictable = true;
		    /* Fall through.  */
		  case 'R':
		    if (value == 13)
		      is_unpredictable = true;
		    /* Fall through.  */
		  case 'r':
		    func (stream, "%s", arm_regnames[value]);
		    break;

		  // <MASK>

		  case 'V':
		    if (given & (1 << 6))
		      func (stream, "q%ld", value >> 1);
		    else if (given & (1 << 24))
		      func (stream, "d%ld", value);
		    else
		      {
			/* Encoding for S register is different than for D and
			   Q registers.  S registers are encoded using the top
			   single bit in position 22 as the lowest bit of the
			   register number, while for Q and D it represents the
			   highest bit of the register number.  */
			uint8_t top_bit = (value >> 4) & 1;
			uint8_t tmp = (value << 1) & 0x1e;
			uint8_t res = tmp | top_bit;
			func (stream, "s%u", res);
		      }
		    break;

		default:
		  abort ();
		}
	      }
	    break;

	    case 'p':
	      {
		uint8_t proc_number = (given >> 8) & 0x7;
		func (stream, "p%u", proc_number);
		break;
	      }

	    case 'a':
	      {
		uint8_t a_offset = 28;
		if (given & (1 << a_offset))
		  func (stream, "a");
		break;
	      }
	  default:
	    abort ();
	  }
	}
	else
	  func (stream, "%c", *c);
      }

      if (is_unpredictable)
	func (stream, UNPREDICTABLE_INSTRUCTION);

      return true;
      }
    }
    return false;
  }
  else
    return false;
}