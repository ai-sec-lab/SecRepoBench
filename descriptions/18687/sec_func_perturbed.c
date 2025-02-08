static int
disassemble (bfd_vma memaddr,
	     struct disassemble_info *info,
	     int bytes_read,
	     unsigned long insn)
{
  struct v850_opcode *op = (struct v850_opcode *) v850_opcodes;
  const struct v850_operand *operand;
  int match = 0;
  int target_processor;

  switch (info->mach)
    {
    case 0:
    default:
      target_processor = PROCESSOR_V850;
      break;

    case bfd_mach_v850e:
      target_processor = PROCESSOR_V850E;
      break;

    case bfd_mach_v850e1:
      target_processor = PROCESSOR_V850E;
      break;

    case bfd_mach_v850e2:
      target_processor = PROCESSOR_V850E2;
      break;

    case bfd_mach_v850e2v3:
      target_processor = PROCESSOR_V850E2V3;
      break;

    case bfd_mach_v850e3v5:
      target_processor = PROCESSOR_V850E3V5;
      break;
    }

  /* If this is a two byte insn, then mask off the high bits.  */
  if (bytes_read == 2)
    insn &= 0xffff;

  /* Find the opcode.  */
  while (op->name)
    {
      if ((op->mask & insn) == op->opcode
	  && (op->processors & target_processor)
	  && !(op->processors & PROCESSOR_OPTION_ALIAS))
	{
	  /* Code check start.  */
	  const unsigned char *opindex_ptr;
	  unsigned int opnum;
	  unsigned int memop;

	  for (opindex_ptr = op->operands, opnum = 1;
	       *opindex_ptr != 0;
	       opindex_ptr++, opnum++)
	    {
	      int invalid = 0;
	      long value;

	      operand = &v850_operands[*opindex_ptr];

	      value = get_operand_value (operand, insn, bytes_read, memaddr,
					 info, 1, &invalid);

	      if (invalid)
		goto next_opcode;

              if ((operand->flags & V850_NOT_R0) && value == 0 && (op->memop) <=2)
		goto next_opcode;

	      if ((operand->flags & V850_NOT_SA) && value == 0xd)
		goto next_opcode;

	      if ((operand->flags & V850_NOT_IMM0) && value == 0)
		goto next_opcode;
	    }

	  /* Code check end.  */

	  match = 1;
	  (*info->fprintf_func) (info->stream, "%s\t", op->name);
#if 0
	  fprintf (stderr, "match: insn: %lx, mask: %lx, opcode: %lx, name: %s\n",
		   insn, op->mask, op->opcode, op->name );
#endif

	  memop = op->memop;
	  /* Now print the operands.

	     MEMOP is the operand number at which a memory
	     address specification starts, or zero if this
	     instruction has no memory addresses.

	     A memory address is always two arguments.

	     This information allows us to determine when to
	     insert commas into the output stream as well as
	     when to insert disp[reg] expressions onto the
	     output stream.  */

	  for (opindex_ptr = op->operands, opnum = 1;
	       *opindex_ptr != 0;
	       opindex_ptr++, opnum++)
	    {
	      bfd_boolean square = FALSE;
	      long value;
	      int indicator;
	      char *prefix;

	      operand = &v850_operands[*opindex_ptr];

	      value = get_operand_value (operand, insn, bytes_read, memaddr,
					 info, 0, 0);

	      /* The first operand is always output without any
		 special handling.

		 For the following arguments:

		   If memop && opnum == memop + 1, then we need '[' since
		   we're about to output the register used in a memory
		   reference.

		   If memop && opnum == memop + 2, then we need ']' since
		   we just finished the register in a memory reference.  We
		   also need a ',' before this operand.

		   Else we just need a comma.

		   We may need to output a trailing ']' if the last operand
		   in an instruction is the register for a memory address.

		   The exception (and there's always an exception) are the
		   "jmp" insn which needs square brackets around it's only
		   register argument, and the clr1/not1/set1/tst1 insns
		   which [...] around their second register argument.  */

	      prefix = "";
	      if (operand->flags & V850_OPERAND_BANG)
		{
		  prefix = "!";
		}
	      else if (operand->flags & V850_OPERAND_PERCENT)
		{
		  prefix = "%";
		}

	      if (opnum == 1 && opnum == memop)
		{
		  info->fprintf_func (info->stream, "%s[", prefix);
		  square = TRUE;
		}
	      else if (   (strcmp ("stc.w", op->name) == 0
			|| strcmp ("cache", op->name) == 0
			|| strcmp ("pref",  op->name) == 0)
		       && opnum == 2 && opnum == memop)
		{
		  info->fprintf_func (info->stream, ", [");
		  square = TRUE;
		}
	      else if (   (strcmp (op->name, "pushsp") == 0
			|| strcmp (op->name, "popsp") == 0
			|| strcmp (op->name, "dbpush" ) == 0)
		       && opnum == 2)
		{
		  info->fprintf_func (info->stream, "-");
		}
	      else if (opnum > 1
		       && (v850_operands[*(opindex_ptr - 1)].flags
			   & V850_OPERAND_DISP) != 0
		       && opnum == memop)
		{
		  info->fprintf_func (info->stream, "%s[", prefix);
		  square = TRUE;
		}
	      else if (opnum == 2
		       && (   op->opcode == 0x00e407e0 /* clr1 */
			   || op->opcode == 0x00e207e0 /* not1 */
			   || op->opcode == 0x00e007e0 /* set1 */
			   || op->opcode == 0x00e607e0 /* tst1 */
			   ))
		{
		  info->fprintf_func (info->stream, ", %s[", prefix);
		  square = TRUE;
		}
	      else if (opnum > 1)
		info->fprintf_func (info->stream, ", %s", prefix);

 	      /* Extract the flags, ignoring ones which do not
		 effect disassembly output.  */
	      indicator = operand->flags & (V850_OPERAND_REG
				       | V850_REG_EVEN
				       | V850_OPERAND_EP
				       | V850_OPERAND_SRG
				       | V850E_OPERAND_REG_LIST
				       | V850_OPERAND_CC
				       | V850_OPERAND_VREG
				       | V850_OPERAND_CACHEOP
				       | V850_OPERAND_PREFOP
				       | V850_OPERAND_FLOAT_CC);

	      switch (indicator)
		{
		case V850_OPERAND_REG:
		  info->fprintf_func (info->stream, "%s", get_v850_reg_name (value));
		  break;
		case (V850_OPERAND_REG|V850_REG_EVEN):
		  info->fprintf_func (info->stream, "%s", get_v850_reg_name (value * 2));
		  break;
		case V850_OPERAND_EP:
		  info->fprintf_func (info->stream, "ep");
		  break;
		case V850_OPERAND_SRG:
		  info->fprintf_func (info->stream, "%s", get_v850_sreg_name (value));
		  break;
		case V850E_OPERAND_REG_LIST:
		  {
		    static int list12_regs[32]   = { 30, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
						     0,  0, 0, 0, 0, 31, 29, 28, 23, 22, 21, 20, 27, 26, 25, 24 };
		    int *regs;
		    int i;
		    unsigned long int mask = 0;
		    int pc = 0;

		    switch (operand->shift)
		      {
		      case 0xffe00001: regs = list12_regs; break;
		      default:
			/* xgettext:c-format */
			opcodes_error_handler (_("unknown operand shift: %x"),
					       operand->shift);
			abort ();
		      }

		    for (i = 0; i < 32; i++)
		      {
			if (value & (1 << i))
			  {
			    switch (regs[ i ])
			      {
			      default:
				mask |= (1 << regs[ i ]);
				break;
			      case 0:
				/* xgettext:c-format */
				opcodes_error_handler (_("unknown reg: %d"), i);
				abort ();
				break;
			      case -1:
				pc = 1;
				break;
			      }
			  }
		      }

		    info->fprintf_func (info->stream, "{");

		    if (mask || pc)
		      {
			if (mask)
			  {
			    unsigned int bit;
			    int shown_one = 0;

			    for (bit = 0; bit < 32; bit++)
			      if (mask & (1 << bit))
				{
				  unsigned long int first = bit;
				  unsigned long int last;

				  if (shown_one)
				    info->fprintf_func (info->stream, ", ");
				  else
				    shown_one = 1;

				  info->fprintf_func (info->stream, "%s", get_v850_reg_name (first));

				  for (bit++; bit < 32; bit++)
				    if ((mask & (1 << bit)) == 0)
				      break;

				  last = bit;

				  if (last > first + 1)
				    {
				      info->fprintf_func (info->stream, " - %s", get_v850_reg_name (last - 1));
				    }
				}
			  }

			if (pc)
			  info->fprintf_func (info->stream, "%sPC", mask ? ", " : "");
		      }

		    info->fprintf_func (info->stream, "}");
		  }
		  break;

		case V850_OPERAND_CC:
		  info->fprintf_func (info->stream, "%s", get_v850_cc_name (value));
		  break;

		case V850_OPERAND_FLOAT_CC:
		  info->fprintf_func (info->stream, "%s", get_v850_float_cc_name (value));
		  break;

		case V850_OPERAND_CACHEOP:
		  {
		    int idx;

		    for (idx = 0; v850_cacheop_codes[idx] != -1; idx++)
		      {
			if (value == v850_cacheop_codes[idx])
			  {
			    info->fprintf_func (info->stream, "%s",
						get_v850_cacheop_name (idx));
			    goto MATCH_CACHEOP_CODE;
			  }
		      }
		    info->fprintf_func (info->stream, "%d", (int) value);
		  }
		MATCH_CACHEOP_CODE:
		  break;

		case V850_OPERAND_PREFOP:
		  {
		    int idx;

		    for (idx = 0; v850_prefop_codes[idx] != -1; idx++)
		      {
			if (value == v850_prefop_codes[idx])
			  {
			    info->fprintf_func (info->stream, "%s",
						get_v850_prefop_name (idx));
			    goto MATCH_PREFOP_CODE;
			  }
		      }
		    info->fprintf_func (info->stream, "%d", (int) value);
		  }
		MATCH_PREFOP_CODE:
		  break;

		case V850_OPERAND_VREG:
		  info->fprintf_func (info->stream, "%s", get_v850_vreg_name (value));
		  break;

		default:
		  print_value (operand->flags, memaddr, info, value);
		  break;
		}

	      if (square)
		(*info->fprintf_func) (info->stream, "]");
	    }

	  /* All done. */
	  break;
	}
    next_opcode:
      op++;
    }

  return match;
}