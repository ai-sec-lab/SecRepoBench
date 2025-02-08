static void
print_insn_arm (bfd_vma pc, struct disassemble_info *info, long given)
{
  const struct opcode32 *insn;
  void *stream = info->stream;
  fprintf_ftype func = info->fprintf_func;
  struct arm_private_data *private_data = info->private_data;

  if (print_insn_coprocessor (pc, info, given, FALSE))
    return;

  if (print_insn_neon (info, given, FALSE))
    return;

  if (print_insn_generic_coprocessor (pc, info, given, FALSE))
    return;

  for (insn = arm_opcodes; insn->assembler; insn++)
    {
      if ((given & insn->mask) != insn->value)
	continue;

      if (! ARM_CPU_HAS_FEATURE (insn->arch, private_data->features))
	continue;

      /* Special case: an instruction with all bits set in the condition field
	 (0xFnnn_nnnn) is only matched if all those bits are set in insn->mask,
	 or by the catchall at the end of the table.  */
      if ((given & 0xF0000000) != 0xF0000000
	  || (insn->mask & 0xF0000000) == 0xF0000000
	  || (insn->mask == 0 && insn->value == 0))
	{
	  unsigned long u_reg = 16;
	  unsigned long U_reg = 16;
	  bfd_boolean is_unpredictable = FALSE;
	  signed long commentvalue = 0;
	  const char *c;

	  for (c = insn->assembler; *c; c++)
	    {
	      if (*c == '%')
		{
		  bfd_boolean allow_unpredictable = FALSE;

		  switch (*++c)
		    {
		    case '%':
		      func (stream, "%%");
		      break;

		    case 'a':
		      commentvalue = print_arm_address (pc, info, given);
		      break;

		    case 'P':
		      /* Set P address bit and use normal address
			 printing routine.  */
		      commentvalue = print_arm_address (pc, info, given | (1 << P_BIT));
		      break;

		    case 'S':
		      allow_unpredictable = TRUE;
		      /* Fall through.  */
		    case 's':
                      if ((given & 0x004f0000) == 0x004f0000)
			{
                          /* PC relative with immediate offset.  */
			  bfd_vma offset = ((given & 0xf00) >> 4) | (given & 0xf);

			  if (PRE_BIT_SET)
			    {
			      /* Elide positive zero offset.  */
			      if (offset || NEGATIVE_BIT_SET)
				func (stream, "[pc, #%s%d]\t; ",
				      NEGATIVE_BIT_SET ? "-" : "", (int) offset);
			      else
				func (stream, "[pc]\t; ");
			      if (NEGATIVE_BIT_SET)
				offset = -offset;
			      info->print_address_func (offset + pc + 8, info);
			    }
			  else
			    {
			      /* Always show the offset.  */
			      func (stream, "[pc], #%s%d",
				    NEGATIVE_BIT_SET ? "-" : "", (int) offset);
			      if (! allow_unpredictable)
				is_unpredictable = TRUE;
			    }
			}
		      else
			{
			  int offset = ((given & 0xf00) >> 4) | (given & 0xf);

			  func (stream, "[%s",
				arm_regnames[(given >> 16) & 0xf]);

			  if (PRE_BIT_SET)
			    {
			      if (IMMEDIATE_BIT_SET)
				{
				  /* Elide offset for non-writeback
				     positive zero.  */
				  if (WRITEBACK_BIT_SET || NEGATIVE_BIT_SET
				      || offset)
				    func (stream, ", #%s%d",
					  NEGATIVE_BIT_SET ? "-" : "", offset);

				  if (NEGATIVE_BIT_SET)
				    offset = -offset;

				  commentvalue = offset;
				}
			      else
				{
				  /* Register Offset or Register Pre-Indexed.  */
				  func (stream, ", %s%s",
					NEGATIVE_BIT_SET ? "-" : "",
					arm_regnames[given & 0xf]);

				  /* Writing back to the register that is the source/
				     destination of the load/store is unpredictable.  */
				  if (! allow_unpredictable
				      && WRITEBACK_BIT_SET
				      && ((given & 0xf) == ((given >> 12) & 0xf)))
				    is_unpredictable = TRUE;
				}

			      func (stream, "]%s",
				    WRITEBACK_BIT_SET ? "!" : "");
			    }
			  else
			    {
			      if (IMMEDIATE_BIT_SET)
				{
				  /* Immediate Post-indexed.  */
				  /* PR 10924: Offset must be printed, even if it is zero.  */
				  func (stream, "], #%s%d",
					NEGATIVE_BIT_SET ? "-" : "", offset);
				  if (NEGATIVE_BIT_SET)
				    offset = -offset;
				  commentvalue = offset;
				}
			      else
				{
				  /* Register Post-indexed.  */
				  func (stream, "], %s%s",
					NEGATIVE_BIT_SET ? "-" : "",
					arm_regnames[given & 0xf]);

				  /* Writing back to the register that is the source/
				     destination of the load/store is unpredictable.  */
				  if (! allow_unpredictable
				      && (given & 0xf) == ((given >> 12) & 0xf))
				    is_unpredictable = TRUE;
				}

			      if (! allow_unpredictable)
				{
				  /* Writeback is automatically implied by post- addressing.
				     Setting the W bit is unnecessary and ARM specify it as
				     being unpredictable.  */
				  if (WRITEBACK_BIT_SET
				      /* Specifying the PC register as the post-indexed
					 registers is also unpredictable.  */
				      || (! IMMEDIATE_BIT_SET && ((given & 0xf) == 0xf)))
				    is_unpredictable = TRUE;
				}
			    }
			}
		      break;

		    case 'b':
		      {
			bfd_vma disp = (((given & 0xffffff) ^ 0x800000) - 0x800000);
			bfd_vma target = disp * 4 + pc + 8;
			info->print_address_func (target, info);

			/* Fill in instruction information.  */
			info->insn_info_valid = 1;
			info->insn_type = dis_branch;
			info->target = target;
		      }
		      break;

		    case 'c':
		      if (((given >> 28) & 0xf) != 0xe)
			func (stream, "%s",
			      arm_conditional [(given >> 28) & 0xf]);
		      break;

		    case 'm':
		      {
			int started = 0;
			int reg;

			func (stream, "{");
			for (reg = 0; reg < 16; reg++)
			  if ((given & (1 << reg)) != 0)
			    {
			      if (started)
				func (stream, ", ");
			      started = 1;
			      func (stream, "%s", arm_regnames[reg]);
			    }
			func (stream, "}");
			if (! started)
			  is_unpredictable = TRUE;
		      }
		      break;

		    case 'q':
		      arm_decode_shift (given, func, stream, FALSE);
		      break;

		    case 'o':
		      if ((given & 0x02000000) != 0)
			{
			  unsigned int rotate = (given & 0xf00) >> 7;
			  unsigned int immed = (given & 0xff);
			  unsigned int a, i;

			  a = (immed << ((32 - rotate) & 31)
			       | immed >> rotate) & 0xffffffff;
			  /* If there is another encoding with smaller rotate,
			     the rotate should be specified directly.  */
			  for (i = 0; i < 32; i += 2)
			    if ((a << i | a >> ((32 - i) & 31)) <= 0xff)
			      break;

			  if (i != rotate)
			    func (stream, "#%d, %d", immed, rotate);
			  else
			    func (stream, "#%d", a);
			  commentvalue = a;
			}
		      else
			arm_decode_shift (given, func, stream, TRUE);
		      break;

		    case 'p':
		      if ((given & 0x0000f000) == 0x0000f000)
			{
			  arm_feature_set arm_ext_v6 =
			    ARM_FEATURE_CORE_LOW (ARM_EXT_V6);

			  /* The p-variants of tst/cmp/cmn/teq are the pre-V6
			     mechanism for setting PSR flag bits.  They are
			     obsolete in V6 onwards.  */
			  if (! ARM_CPU_HAS_FEATURE (private_data->features, \
						     arm_ext_v6))
			    func (stream, "p");
			  else
			    is_unpredictable = TRUE;
			}
		      break;

		    case 't':
		      if ((given & 0x01200000) == 0x00200000)
			func (stream, "t");
		      break;

		    case 'A':
		      {
			int offset = given & 0xff;

			commentvalue = offset * 4;
			if (NEGATIVE_BIT_SET)
			  commentvalue = - commentvalue;

			func (stream, "[%s", arm_regnames [(given >> 16) & 0xf]);

			if (PRE_BIT_SET)
			  {
			    if (offset)
			      func (stream, ", #%d]%s",
				    (int) commentvalue,
				    WRITEBACK_BIT_SET ? "!" : "");
			    else
			      func (stream, "]");
			  }
			else
			  {
			    func (stream, "]");

			    if (WRITEBACK_BIT_SET)
			      {
				if (offset)
				  func (stream, ", #%d", (int) commentvalue);
			      }
			    else
			      {
				func (stream, ", {%d}", (int) offset);
				commentvalue = offset;
			      }
			  }
		      }
		      break;

		    case 'B':
		      /* Print ARM V5 BLX(1) address: pc+25 bits.  */
		      {
			bfd_vma address;
			bfd_vma offset = 0;

			if (! NEGATIVE_BIT_SET)
			  /* Is signed, hi bits should be ones.  */
			  offset = (-1) ^ 0x00ffffff;

			/* Offset is (SignExtend(offset field)<<2).  */
			offset += given & 0x00ffffff;
			offset <<= 2;
			address = offset + pc + 8;

			if (given & 0x01000000)
			  /* H bit allows addressing to 2-byte boundaries.  */
			  address += 2;

		        info->print_address_func (address, info);

			/* Fill in instruction information.  */
			info->insn_info_valid = 1;
			info->insn_type = dis_branch;
			info->target = address;
		      }
		      break;

		    case 'C':
		      if ((given & 0x02000200) == 0x200)
			{
			  const char * name;
			  unsigned sysm = (given & 0x004f0000) >> 16;

			  sysm |= (given & 0x300) >> 4;
			  name = banked_regname (sysm);

			  if (name != NULL)
			    func (stream, "%s", name);
			  else
			    func (stream, "(UNDEF: %lu)", (unsigned long) sysm);
			}
		      else
			{
			  func (stream, "%cPSR_",
				(given & 0x00400000) ? 'S' : 'C');
			  if (given & 0x80000)
			    func (stream, "f");
			  if (given & 0x40000)
			    func (stream, "s");
			  if (given & 0x20000)
			    func (stream, "x");
			  if (given & 0x10000)
			    func (stream, "c");
			}
		      break;

		    case 'U':
		      if ((given & 0xf0) == 0x60)
			{
			  switch (given & 0xf)
			    {
			    case 0xf: func (stream, "sy"); break;
			    default:
			      func (stream, "#%d", (int) given & 0xf);
			      break;
			    }
			}
		      else
			{
			  const char * opt = data_barrier_option (given & 0xf);
			  if (opt != NULL)
			    func (stream, "%s", opt);
			  else
			      func (stream, "#%d", (int) given & 0xf);
			}
		      break;

		    case '0': case '1': case '2': case '3': case '4':
		    case '5': case '6': case '7': case '8': case '9':
		      {
			int width;
			unsigned long value;

			c = arm_decode_bitfield (c, given, &value, &width);

			switch (*c)
			  {
			  case 'R':
			    if (value == 15)
			      is_unpredictable = TRUE;
			    /* Fall through.  */
			  case 'r':
			  case 'T'// <MASK>
			    if (c[1] == 'U')
			      {
				/* Eat the 'U' character.  */
				++ c;

				if (U_reg == value)
				  is_unpredictable = TRUE;
				U_reg = value;
			      }
			    func (stream, "%s", arm_regnames[value]);
			    break;
			  case 'd':
			    func (stream, "%ld", value);
			    commentvalue = value;
			    break;
			  case 'b':
			    func (stream, "%ld", value * 8);
			    commentvalue = value * 8;
			    break;
			  case 'W':
			    func (stream, "%ld", value + 1);
			    commentvalue = value + 1;
			    break;
			  case 'x':
			    func (stream, "0x%08lx", value);

			    /* Some SWI instructions have special
			       meanings.  */
			    if ((given & 0x0fffffff) == 0x0FF00000)
			      func (stream, "\t; IMB");
			    else if ((given & 0x0fffffff) == 0x0FF00001)
			      func (stream, "\t; IMBRange");
			    break;
			  case 'X':
			    func (stream, "%01lx", value & 0xf);
			    commentvalue = value;
			    break;
			  case '`':
			    c++;
			    if (value == 0)
			      func (stream, "%c", *c);
			    break;
			  case '\'':
			    c++;
			    if (value == ((1ul << width) - 1))
			      func (stream, "%c", *c);
			    break;
			  case '?':
			    func (stream, "%c", c[(1 << width) - (int) value]);
			    c += 1 << width;
			    break;
			  default:
			    abort ();
			  }
		      }
		      break;

		    case 'e':
		      {
			int imm;

			imm = (given & 0xf) | ((given & 0xfff00) >> 4);
			func (stream, "%d", imm);
			commentvalue = imm;
		      }
		      break;

		    case 'E':
		      /* LSB and WIDTH fields of BFI or BFC.  The machine-
			 language instruction encodes LSB and MSB.  */
		      {
			long msb = (given & 0x001f0000) >> 16;
			long lsb = (given & 0x00000f80) >> 7;
			long w = msb - lsb + 1;

			if (w > 0)
			  func (stream, "#%lu, #%lu", lsb, w);
			else
			  func (stream, "(invalid: %lu:%lu)", lsb, msb);
		      }
		      break;

		    case 'R':
		      /* Get the PSR/banked register name.  */
		      {
			const char * name;
			unsigned sysm = (given & 0x004f0000) >> 16;

			sysm |= (given & 0x300) >> 4;
			name = banked_regname (sysm);

			if (name != NULL)
			  func (stream, "%s", name);
			else
			  func (stream, "(UNDEF: %lu)", (unsigned long) sysm);
		      }
		      break;

		    case 'V':
		      /* 16-bit unsigned immediate from a MOVT or MOVW
			 instruction, encoded in bits 0:11 and 15:19.  */
		      {
			long hi = (given & 0x000f0000) >> 4;
			long lo = (given & 0x00000fff);
			long imm16 = hi | lo;

			func (stream, "#%lu", imm16);
			commentvalue = imm16;
		      }
		      break;

		    default:
		      abort ();
		    }
		}
	      else
		func (stream, "%c", *c);
	    }

	  if (commentvalue > 32 || commentvalue < -16)
	    func (stream, "\t; 0x%lx", (commentvalue & 0xffffffffUL));

	  if (is_unpredictable)
	    func (stream, UNPREDICTABLE_INSTRUCTION);

	  return;
	}
    }
  func (stream, UNKNOWN_INSTRUCTION_32BIT, (unsigned)given);
  return;
}