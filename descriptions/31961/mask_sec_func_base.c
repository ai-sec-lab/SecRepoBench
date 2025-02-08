static int
print_insn (bfd_vma pc, disassemble_info *info)
{
  const struct dis386 *dp;
  int i;
  char *op_txt[MAX_OPERANDS];
  int needcomma;
  int sizeflag, orig_sizeflag;
  const char *p;
  struct dis_private priv;
  int prefix_length;

  priv.orig_sizeflag = AFLAG | DFLAG;
  if ((info->mach & bfd_mach_i386_i386) != 0)
    address_mode = mode_32bit;
  else if (info->mach == bfd_mach_i386_i8086)
    {
      address_mode = mode_16bit;
      priv.orig_sizeflag = 0;
    }
  else
    address_mode = mode_64bit;

  if (intel_syntax == (char) -1)
    intel_syntax = (info->mach & bfd_mach_i386_intel_syntax) != 0;

  for (p = info->disassembler_options; p != NULL; )
    {
      if (CONST_STRNEQ (p, "amd64"))
	isa64 = amd64;
      else if (CONST_STRNEQ (p, "intel64"))
	isa64 = intel64;
      else if (CONST_STRNEQ (p, "x86-64"))
	{
	  address_mode = mode_64bit;
	  priv.orig_sizeflag |= AFLAG | DFLAG;
	}
      else if (CONST_STRNEQ (p, "i386"))
	{
	  address_mode = mode_32bit;
	  priv.orig_sizeflag |= AFLAG | DFLAG;
	}
      else if (CONST_STRNEQ (p, "i8086"))
	{
	  address_mode = mode_16bit;
	  priv.orig_sizeflag &= ~(AFLAG | DFLAG);
	}
      else if (CONST_STRNEQ (p, "intel"))
	{
	  intel_syntax = 1;
	  if (CONST_STRNEQ (p + 5, "-mnemonic"))
	    intel_mnemonic = 1;
	}
      else if (CONST_STRNEQ (p, "att"))
	{
	  intel_syntax = 0;
	  if (CONST_STRNEQ (p + 3, "-mnemonic"))
	    intel_mnemonic = 0;
	}
      else if (CONST_STRNEQ (p, "addr"))
	{
	  if (address_mode == mode_64bit)
	    {
	      if (p[4] == '3' && p[5] == '2')
		priv.orig_sizeflag &= ~AFLAG;
	      else if (p[4] == '6' && p[5] == '4')
		priv.orig_sizeflag |= AFLAG;
	    }
	  else
	    {
	      if (p[4] == '1' && p[5] == '6')
		priv.orig_sizeflag &= ~AFLAG;
	      else if (p[4] == '3' && p[5] == '2')
		priv.orig_sizeflag |= AFLAG;
	    }
	}
      else if (CONST_STRNEQ (p, "data"))
	{
	  if (p[4] == '1' && p[5] == '6')
	    priv.orig_sizeflag &= ~DFLAG;
	  else if (p[4] == '3' && p[5] == '2')
	    priv.orig_sizeflag |= DFLAG;
	}
      else if (CONST_STRNEQ (p, "suffix"))
	priv.orig_sizeflag |= SUFFIX_ALWAYS;

      p = strchr (p, ',');
      if (p != NULL)
	p++;
    }

  if (address_mode == mode_64bit && sizeof (bfd_vma) < 8)
    {
      (*info->fprintf_func) (info->stream,
			     _("64-bit address is disabled"));
      return -1;
    }

  if (intel_syntax)
    {
      names64 = intel_names64;
      names32 = intel_names32;
      names16 = intel_names16;
      names8 = intel_names8;
      names8rex = intel_names8rex;
      names_seg = intel_names_seg;
      names_mm = intel_names_mm;
      names_bnd = intel_names_bnd;
      names_xmm = intel_names_xmm;
      names_ymm = intel_names_ymm;
      names_zmm = intel_names_zmm;
      names_tmm = intel_names_tmm;
      index64 = intel_index64;
      index32 = intel_index32;
      names_mask = intel_names_mask;
      index16 = intel_index16;
      open_char = '[';
      close_char = ']';
      separator_char = '+';
      scale_char = '*';
    }
  else
    {
      names64 = att_names64;
      names32 = att_names32;
      names16 = att_names16;
      names8 = att_names8;
      names8rex = att_names8rex;
      names_seg = att_names_seg;
      names_mm = att_names_mm;
      names_bnd = att_names_bnd;
      names_xmm = att_names_xmm;
      names_ymm = att_names_ymm;
      names_zmm = att_names_zmm;
      names_tmm = att_names_tmm;
      index64 = att_index64;
      index32 = att_index32;
      names_mask = att_names_mask;
      index16 = att_index16;
      open_char = '(';
      close_char =  ')';
      separator_char = ',';
      scale_char = ',';
    }

  /* The output looks better if we put 7 bytes on a line, since that
     puts most long word instructions on a single line.  Use 8 bytes
     for Intel L1OM.  */
  if ((info->mach & bfd_mach_l1om) != 0)
    info->bytes_per_line = 8;
  else
    info->bytes_per_line = 7;

  info->private_data = &priv;
  priv.max_fetched = priv.the_buffer;
  priv.insn_start = pc;

  obuf[0] = 0;
  for (i = 0; i < MAX_OPERANDS; ++i)
    {
      op_out[i][0] = 0;
      op_index[i] = -1;
    }

  the_info = info;
  start_pc = pc;
  start_codep = priv.the_buffer;
  codep = priv.the_buffer;

  if (OPCODES_SIGSETJMP (priv.bailout) != 0)
    {
      const char *name;

      /* Getting here means we tried for data but didn't get it.  That
	 means we have an incomplete instruction of some sort.  Just
	 print the first byte as a prefix or a .byte pseudo-op.  */
      if (codep > priv.the_buffer)
	{
	  name = prefix_name (priv.the_buffer[0], priv.orig_sizeflag);
	  if (name != NULL)
	    (*info->fprintf_func) (info->stream, "%s", name);
	  else
	    {
	      /* Just print the first byte as a .byte instruction.  */
	      (*info->fprintf_func) (info->stream, ".byte 0x%x",
				     (unsigned int) priv.the_buffer[0]);
	    }

	  return 1;
	}

      return -1;
    }

  obufp = obuf;
  sizeflag = priv.orig_sizeflag;

  if (!ckprefix () || rex_used)
    {
      /* Too many prefixes or unused REX prefixes.  */
      for (i = 0;
	   i < (int) ARRAY_SIZE (all_prefixes) && all_prefixes[i];
	   i++)
	(*info->fprintf_func) (info->stream, "%s%s",
			       i == 0 ? "" : " ",
			       prefix_name (all_prefixes[i], sizeflag));
      return i;
    }

  insn_codep = codep;

  FETCH_DATA (info, codep + 1);
  two_source_ops = (*codep == 0x62) || (*codep == 0xc8);

  if (((prefixes & PREFIX_FWAIT)
       && ((*codep < 0xd8) || (*codep > 0xdf))))
    {
      /* Handle prefixes before fwait.  */
      for (i = 0; i < fwait_prefix && all_prefixes[i];
	   i++)
	(*info->fprintf_func) (info->stream, "%s ",
			       prefix_name (all_prefixes[i], sizeflag));
      (*info->fprintf_func) (info->stream, "fwait");
      return i + 1;
    }

  if (*codep == 0x0f)
    {
      unsigned char threebyte;

      codep++;
      FETCH_DATA (info, codep + 1);
      threebyte = *codep;
      dp = &dis386_twobyte[threebyte];
      need_modrm = twobyte_has_modrm[threebyte];
      codep++;
    }
  else
    {
      dp = &dis386[*codep];
      need_modrm = onebyte_has_modrm[*codep];
      codep++;
    }

  /* Save sizeflag for printing the extra prefixes later before updating
     it for mnemonic and operand processing.  The prefix names depend
     only on the address mode.  */
  orig_sizeflag = sizeflag;
  if (prefixes & PREFIX_ADDR)
    sizeflag ^= AFLAG;
  if ((prefixes & PREFIX_DATA))
    sizeflag ^= DFLAG;

  end_codep = codep;
  if (need_modrm)
    {
      FETCH_DATA (info, codep + 1);
      modrm.mod = (*codep >> 6) & 3;
      modrm.reg = (*codep >> 3) & 7;
      modrm.rm = *codep & 7;
    }
  else
    memset (&modrm, 0, sizeof (modrm));

  need_vex = 0;
  memset (&vex, 0, sizeof (vex));

  if (dp->name == NULL && dp->op[0].bytemode == FLOATCODE)
    {
      get_sib (info, sizeflag);
      dofloat (sizeflag);
    }
  else
    {
      dp = get_valid_dis386 (dp, info);
      if (dp != NULL && putop (dp->name, sizeflag) == 0)
	{
	  get_sib (info, sizeflag);
	  for (i = 0; i < MAX_OPERANDS; ++i)
	    {
	      obufp = op_out[i];
	      op_ad = MAX_OPERANDS - 1 - i;
	      if (dp->op[i].rtn)
		(*dp->op[i].rtn) (dp->op[i].bytemode, sizeflag);
	      /* For EVEX instruction after the last operand masking
		 should be printed.  */
	      if (i == 0 && vex.evex)
		{
		  /* Don't print {%k0}.  */
		  if (vex.mask_register_specifier)
		    {
		      oappend ("{");
		      oappend (names_mask[vex.mask_register_specifier]);
		      oappend ("}");
		    }
		  if (vex.zeroing)
		    oappend ("{z}");
		}
	    }
	}
    }

  /* Clear instruction information.  */
  if (the_info)
    {
      the_info->insn_info_valid = 0;
      the_info->branch_delay_insns = 0;
      the_info->data_size = 0;
      the_info->insn_type = dis_noninsn;
      the_info->target = 0;
      the_info->target2 = 0;
    }

  /* Reset jump operation indicator.  */
  op_is_jump = FALSE;

  {
    int jump_detection = 0;

    /* Extract flags.  */
    for (i = 0; i < MAX_OPERANDS; ++i)
      {
	if ((dp->op[i].rtn == OP_J)
	    || (dp->op[i].rtn == OP_indirE))
	  jump_detection |= 1;
	else if ((dp->op[i].rtn == BND_Fixup)
		 || (!dp->op[i].rtn && !dp->op[i].bytemode))
	  jump_detection |= 2;
	else if ((dp->op[i].bytemode == cond_jump_mode)
		 || (dp->op[i].bytemode == loop_jcxz_mode))
	  jump_detection |= 4;
      }

    /* Determine if this is a jump or branch.  */
    if ((jump_detection & 0x3) == 0x3)
      {
	op_is_jump = TRUE;
	if (jump_detection & 0x4)
	  the_info->insn_type = dis_condbranch;
	else
	  the_info->insn_type =
	    (dp->name && !strncmp(dp->name, "call", 4))
	    ? dis_jsr : dis_branch;
      }
  }

  /* If VEX.vvvv and EVEX.vvvv are unused, they must be all 1s, which
     are all 0s in inverted form.  */
  if (need_vex && vex.register_specifier != 0)
    {
      (*info->fprintf_func) (info->stream, "(bad)");
      return end_codep - priv.the_buffer;
    }

  switch (dp->prefix_requirement)
    {
    case PREFIX_DATA:
      /* If only the data prefix is marked as mandatory, its absence renders
	 the encoding invalid.  Most other PREFIX_OPCODE rules still apply.  */
      if (need_vex ? !vex.prefix : !(prefixes & PREFIX_DATA))
	{
	  (*info->fprintf_func) (info->stream, "(bad)");
	  return end_codep - priv.the_buffer;
	}
      used_prefixes |= PREFIX_DATA;
      /* Fall through.  */
    case PREFIX_OPCODE:
      /* If the mandatory PREFIX_REPZ/PREFIX_REPNZ/PREFIX_DATA prefix is
	 unused, opcode is invalid.  Since the PREFIX_DATA prefix may be
	 used by putop and MMX/SSE operand and may be overridden by the
	 PREFIX_REPZ/PREFIX_REPNZ fix, we check the PREFIX_DATA prefix
	 separately.  */
      if (((need_vex
	    ? vex.prefix == REPE_PREFIX_OPCODE
	      || vex.prefix == REPNE_PREFIX_OPCODE
	    : (prefixes
	       & (PREFIX_REPZ | PREFIX_REPNZ)) != 0)
	   && (used_prefixes
	       & (PREFIX_REPZ | PREFIX_REPNZ)) == 0)
	  || (((need_vex
		? vex.prefix == DATA_PREFIX_OPCODE
		: ((prefixes
		    & (PREFIX_REPZ | PREFIX_REPNZ | PREFIX_DATA))
		   == PREFIX_DATA))
	       && (used_prefixes & PREFIX_DATA) == 0))
	  || (vex.evex && dp->prefix_requirement != PREFIX_DATA
	      && !vex.w != !(used_prefixes & PREFIX_DATA)))
	{
	  (*info->fprintf_func) (info->stream, "(bad)");
	  return end_codep - priv.the_buffer;
	}
      break;

    case PREFIX_IGNORED:
      // <MASK>
      break;
    }

  /* Check if the REX prefix is used.  */
  if ((rex ^ rex_used) == 0 && !need_vex && last_rex_prefix >= 0)
    all_prefixes[last_rex_prefix] = 0;

  /* Check if the SEG prefix is used.  */
  if ((prefixes & (PREFIX_CS | PREFIX_SS | PREFIX_DS | PREFIX_ES
		   | PREFIX_FS | PREFIX_GS)) != 0
      && (used_prefixes & active_seg_prefix) != 0)
    all_prefixes[last_seg_prefix] = 0;

  /* Check if the ADDR prefix is used.  */
  if ((prefixes & PREFIX_ADDR) != 0
      && (used_prefixes & PREFIX_ADDR) != 0)
    all_prefixes[last_addr_prefix] = 0;

  /* Check if the DATA prefix is used.  */
  if ((prefixes & PREFIX_DATA) != 0
      && (used_prefixes & PREFIX_DATA) != 0
      && !need_vex)
    all_prefixes[last_data_prefix] = 0;

  /* Print the extra prefixes.  */
  prefix_length = 0;
  for (i = 0; i < (int) ARRAY_SIZE (all_prefixes); i++)
    if (all_prefixes[i])
      {
	const char *name;
	name = prefix_name (all_prefixes[i], orig_sizeflag);
	if (name == NULL)
	  abort ();
	prefix_length += strlen (name) + 1;
	(*info->fprintf_func) (info->stream, "%s ", name);
      }

  /* Check maximum code length.  */
  if ((codep - start_codep) > MAX_CODE_LENGTH)
    {
      (*info->fprintf_func) (info->stream, "(bad)");
      return MAX_CODE_LENGTH;
    }

  obufp = mnemonicendp;
  for (i = strlen (obuf) + prefix_length; i < 6; i++)
    oappend (" ");
  oappend (" ");
  (*info->fprintf_func) (info->stream, "%s", obuf);

  /* The enter and bound instructions are printed with operands in the same
     order as the intel book; everything else is printed in reverse order.  */
  if (intel_syntax || two_source_ops)
    {
      bfd_vma riprel;

      for (i = 0; i < MAX_OPERANDS; ++i)
	op_txt[i] = op_out[i];

      if (intel_syntax && dp && dp->op[2].rtn == OP_Rounding
          && dp->op[3].rtn == OP_E && dp->op[4].rtn == NULL)
	{
	  op_txt[2] = op_out[3];
	  op_txt[3] = op_out[2];
	}

      for (i = 0; i < (MAX_OPERANDS >> 1); ++i)
	{
	  op_ad = op_index[i];
	  op_index[i] = op_index[MAX_OPERANDS - 1 - i];
	  op_index[MAX_OPERANDS - 1 - i] = op_ad;
	  riprel = op_riprel[i];
	  op_riprel[i] = op_riprel [MAX_OPERANDS - 1 - i];
	  op_riprel[MAX_OPERANDS - 1 - i] = riprel;
	}
    }
  else
    {
      for (i = 0; i < MAX_OPERANDS; ++i)
	op_txt[MAX_OPERANDS - 1 - i] = op_out[i];
    }

  needcomma = 0;
  for (i = 0; i < MAX_OPERANDS; ++i)
    if (*op_txt[i])
      {
	if (needcomma)
	  (*info->fprintf_func) (info->stream, ",");
	if (op_index[i] != -1 && !op_riprel[i])
	  {
	    bfd_vma target = (bfd_vma) op_address[op_index[i]];

	    if (the_info && op_is_jump)
	      {
		the_info->insn_info_valid = 1;
		the_info->branch_delay_insns = 0;
		the_info->data_size = 0;
		the_info->target = target;
		the_info->target2 = 0;
	      }
	    (*info->print_address_func) (target, info);
	  }
	else
	  (*info->fprintf_func) (info->stream, "%s", op_txt[i]);
	needcomma = 1;
      }

  for (i = 0; i < MAX_OPERANDS; i++)
    if (op_index[i] != -1 && op_riprel[i])
      {
	(*info->fprintf_func) (info->stream, "        # ");
	(*info->print_address_func) ((bfd_vma) (start_pc + (codep - start_codep)
						+ op_address[op_index[i]]), info);
	break;
      }
  return codep - priv.the_buffer;
}