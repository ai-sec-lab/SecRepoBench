static int
print_insn_arc (bfd_vma memaddr,
		struct disassemble_info *info)
{
  bfd_byte buffer[8];
  unsigned int msb, lowbyte;
  int status;
  unsigned int insn_len;
  unsigned long long insn = 0;
  unsigned isa_mask = ARC_OPCODE_NONE;
  const struct arc_opcode *opcode;
  bfd_boolean need_comma;
  bfd_boolean open_braket;
  int size;
  const struct arc_operand *operand;
  int value, vpcl;
  struct arc_operand_iterator iter;
  struct arc_disassemble_info *arc_infop;
  bfd_boolean rpcl = FALSE, rset = FALSE;

  if (info->disassembler_options)
    {
      parse_disassembler_options (info->disassembler_options);

      /* Avoid repeated parsing of the options.  */
      info->disassembler_options = NULL;
    }

  if (info->private_data == NULL && !init_arc_disasm_info (info))
    return -1;

  memset (&iter, 0, sizeof (iter));
  msb  = ((info->endian == BFD_ENDIAN_LITTLE) ? 1 : 0);
  lowbyte = ((info->endian == BFD_ENDIAN_LITTLE) ? 0 : 1);

  /* Figure out CPU type, unless it was enforced via disassembler options.  */
  if (enforced_isa_mask == ARC_OPCODE_NONE)
    {
      Elf_Internal_Ehdr *header = NULL;

      if (info->section && info->section->owner)
	header = elf_elfheader (info->section->owner);

      switch (info->mach)
	{
	case bfd_mach_arc_arc700:
	  isa_mask = ARC_OPCODE_ARC700;
	  break;

	case bfd_mach_arc_arc600:
	  isa_mask = ARC_OPCODE_ARC600;
	  break;

	case bfd_mach_arc_arcv2:
	default:
	  isa_mask = ARC_OPCODE_ARCv2EM;
	  /* TODO: Perhaps remove definition of header since it is only used at
	     this location.  */
	  if (header != NULL
	      && (header->e_flags & EF_ARC_MACH_MSK) == EF_ARC_CPU_ARCV2HS)
	    isa_mask = ARC_OPCODE_ARCv2HS;
	  break;
	}
    }
  else
    isa_mask = enforced_isa_mask;

  if (isa_mask == ARC_OPCODE_ARCv2HS)
    {
      /* FPU instructions are not extensions for HS.  */
      add_to_decodelist (FLOAT, SP);
      add_to_decodelist (FLOAT, DP);
      add_to_decodelist (FLOAT, CVT);
    }

  /* This variable may be set by the instruction decoder.  It suggests
     the number of bytes objdump should display on a single line.  If
     the instruction decoder sets this, it should always set it to
     the same value in order to get reasonable looking output.  */
  info->bytes_per_line  = 8;

  /* In the next lines, we set two info variables control the way
     objdump displays the raw data.  For example, if bytes_per_line is
     8 and bytes_per_chunk is 4, the output will look like this:
     00:   00000000 00000000
     with the chunks displayed according to "display_endian".  */
  if (info->section
      && !(info->section->flags & SEC_CODE))
    {
      /* This is not a CODE section.  */
      switch (info->section->size)
	{
	case 1:
	case 2:
	case 4:
	  size = info->section->size;
	  break;
	default:
	  size = (info->section->size & 0x01) ? 1 : 4;
	  break;
	}
      info->bytes_per_chunk = 1;
      info->display_endian = info->endian;
    }
  else
    {
      size = 2;
      info->bytes_per_chunk = 2;
      info->display_endian = info->endian;
    }

  /* Read the insn into a host word.  */
  status = (*info->read_memory_func) (memaddr, buffer, size, info);

  if (status != 0)
    {
      (*info->memory_error_func) (status, memaddr, info);
      return -1;
    }

  if (info->section
      && !(info->section->flags & SEC_CODE))
    {
      /* Data section.  */
      unsigned long data;

      data = bfd_get_bits (buffer, size * 8,
			   info->display_endian == BFD_ENDIAN_BIG);
      switch (size)
	{
	case 1:
	  (*info->fprintf_func) (info->stream, ".byte\t0x%02lx", data);
	  break;
	case 2:
	  (*info->fprintf_func) (info->stream, ".short\t0x%04lx", data);
	  break;
	case 4:
	  (*info->fprintf_func) (info->stream, ".word\t0x%08lx", data);
	  break;
	default:
	  return -1;
	}
      return size;
    }

  insn_len = arc_insn_length (buffer[msb], buffer[lowbyte], info);
  pr_debug ("instruction length = %d bytes\n", insn_len);
  if (insn_len == 0)
    return -1;

  arc_infop = info->private_data;
  arc_infop->insn_len = insn_len;

  switch (insn_len)
    {
    case 2:
      insn = (buffer[msb] << 8) | buffer[lowbyte];
      break;

    case 4:
      {
	/* This is a long instruction: Read the remaning 2 bytes.  */
	status = (*info->read_memory_func) (memaddr + 2, &buffer[2], 2, info);
	if (status != 0)
	  {
	    (*info->memory_error_func) (status, memaddr + 2, info);
	    return -1;
	  }
	insn = (unsigned long long) ARRANGE_ENDIAN (info, buffer);
      }
      break;

    case 6:
      {
	status = (*info->read_memory_func) (memaddr + 2, &buffer[2], 4, info);
	if (status != 0)
	  {
	    (*info->memory_error_func) (status, memaddr + 2, info);
	    return -1;
	  }
	insn = (unsigned long long) ARRANGE_ENDIAN (info, &buffer[2]);
	insn |= ((unsigned long long) buffer[msb] << 40)
	  | ((unsigned long long) buffer[lowbyte] << 32);
      }
      break;

    case 8:
      {
	status = (*info->read_memory_func) (memaddr + 2, &buffer[2], 6, info);
	if (status != 0)
	  {
	    (*info->memory_error_func) (status, memaddr + 2, info);
	    return -1;
	  }
	insn =
	  ((((unsigned long long) ARRANGE_ENDIAN (info, buffer)) << 32)
	   | ((unsigned long long) ARRANGE_ENDIAN (info, &buffer[4])));
      }
      break;

    default:
      /* There is no instruction whose length is not 2, 4, 6, or 8.  */
      return -1;
    }

  pr_debug ("instruction value = %llx\n", insn);

  /* Set some defaults for the insn info.  */
  info->insn_info_valid    = 1;
  info->branch_delay_insns = 0;
  info->data_size	   = 4;
  info->insn_type	   = dis_nonbranch;
  info->target		   = 0;
  info->target2		   = 0;

  /* FIXME to be moved in dissasemble_init_for_target.  */
  info->disassembler_needs_relocs = TRUE;

  /* Find the first match in the opcode table.  */
  if (!find_format (memaddr, insn, &insn_len, isa_mask, info, &opcode, &iter))
    return -1;

  if (!opcode)
    {
      switch (insn_len)
	{
	case 2:
	  (*info->fprintf_func) (info->stream, ".shor\t%#04llx",
				 insn & 0xffff);
	  break;

	case 4:
	  (*info->fprintf_func) (info->stream, ".word\t%#08llx",
				 insn & 0xffffffff);
	  break;

	case 6:
	  (*info->fprintf_func) (info->stream, ".long\t%#08llx",
				 insn & 0xffffffff);
	  (*info->fprintf_func) (info->stream, ".long\t%#04llx",
				 (insn >> 32) & 0xffff);
	  break;

	case 8:
	  (*info->fprintf_func) (info->stream, ".long\t%#08llx",
				 insn & 0xffffffff);
	  (*info->fprintf_func) (info->stream, ".long\t%#08llx",
				 insn >> 32);
	  break;

	default:
	  return -1;
	}

      info->insn_type = dis_noninsn;
      return insn_len;
    }

  /* Print the mnemonic.  */
  (*info->fprintf_func) (info->stream, "%s", opcode->name);

  /* Preselect the insn class.  */
  info->insn_type = arc_opcode_to_insn_type (opcode);

  pr_debug ("%s: 0x%08llx\n", opcode->name, opcode->opcode);

  print_flags (opcode, &insn, info);

  if (opcode->operands[0] != 0)
    (*info->fprintf_func) (info->stream, "\t");

  need_comma = FALSE;
  open_braket = FALSE;
  arc_infop->operands_count = 0;

  /* Now extract and print the operands.  */
  operand = NULL;
  vpcl = 0;
  while (operand_iterator_next (&iter, &operand, &value))
    {
      if (open_braket && (operand->flags & ARC_OPERAND_BRAKET))
	{
	  (*info->fprintf_func) (info->stream, "]");
	  open_braket = FALSE;
	  continue;
	}

      /* Only take input from real operands.  */
      if (ARC_OPERAND_IS_FAKE (operand))
	continue;

      if ((operand->flags & ARC_OPERAND_IGNORE)
	  && (operand->flags & ARC_OPERAND_IR)
	  && value == -1)
	continue;

      if (operand->flags & ARC_OPERAND_COLON)
	{
	  (*info->fprintf_func) (info->stream, ":");
	  continue;
	}

      if (need_comma)
	(*info->fprintf_func) (info->stream, ",");

      if (!open_braket && (operand->flags & ARC_OPERAND_BRAKET))
	{
	  (*info->fprintf_func) (info->stream, "[");
	  open_braket = TRUE;
	  need_comma = FALSE;
	  continue;
	}

      need_comma = TRUE;

      if (operand->flags & ARC_OPERAND_PCREL)
	{
	  rpcl = TRUE;
	  vpcl = value;
	  rset = TRUE;

	  info->target = (bfd_vma) (memaddr & ~3) + value;
	}
      else if (!(operand->flags & ARC_OPERAND_IR))
	{
	  vpcl = value;
	  rset = TRUE;
	}

      /* Print the operand as directed by the flags.  */
      if (operand->flags & ARC_OPERAND_IR)
	{
	  const char *rname;

	  assert (value >=0 && value < 64);
	  rname = arcExtMap_coreRegName (value);
	  if (!rname)
	    rname = regnames[value];
	  (*info->fprintf_func) (info->stream, "%s", rname);

	  /* Check if we have a double register to print.  */
	  if (operand->flags & ARC_OPERAND_TRUNCATE)
	    {
	      // <MASK>
	    }
	  if (value == 63)
	    rpcl = TRUE;
	  else
	    rpcl = FALSE;
	}
      else if (operand->flags & ARC_OPERAND_LIMM)
	{
	  const char *rname = get_auxreg (opcode, value, isa_mask);

	  if (rname && open_braket)
	    (*info->fprintf_func) (info->stream, "%s", rname);
	  else
	    {
	      (*info->fprintf_func) (info->stream, "%#x", value);
	      if (info->insn_type == dis_branch
		  || info->insn_type == dis_jsr)
		info->target = (bfd_vma) value;
	    }
	}
      else if (operand->flags & ARC_OPERAND_SIGNED)
	{
	  const char *rname = get_auxreg (opcode, value, isa_mask);
	  if (rname && open_braket)
	    (*info->fprintf_func) (info->stream, "%s", rname);
	  else
	    {
	      if (print_hex)
		(*info->fprintf_func) (info->stream, "%#x", value);
	      else
		(*info->fprintf_func) (info->stream, "%d", value);
	    }
	}
      else if (operand->flags & ARC_OPERAND_ADDRTYPE)
	{
	  const char *addrtype = get_addrtype (value);
	  (*info->fprintf_func) (info->stream, "%s", addrtype);
	  /* A colon follow an address type.  */
	  need_comma = FALSE;
	}
      else
	{
	  if (operand->flags & ARC_OPERAND_TRUNCATE
	      && !(operand->flags & ARC_OPERAND_ALIGNED32)
	      && !(operand->flags & ARC_OPERAND_ALIGNED16)
	      && value >= 0 && value <= 14)
	    {
	      /* Leave/Enter mnemonics.  */
	      switch (value)
		{
		case 0:
		  need_comma = FALSE;
		  break;
		case 1:
		  (*info->fprintf_func) (info->stream, "r13");
		  break;
		default:
		  (*info->fprintf_func) (info->stream, "r13-%s",
					 regnames[13 + value - 1]);
		  break;
		}
	      rpcl = FALSE;
	      rset = FALSE;
	    }
	  else
	    {
	      const char *rname = get_auxreg (opcode, value, isa_mask);
	      if (rname && open_braket)
		(*info->fprintf_func) (info->stream, "%s", rname);
	      else
		(*info->fprintf_func) (info->stream, "%#x", value);
	    }
	}

      if (operand->flags & ARC_OPERAND_LIMM)
	{
	  arc_infop->operands[arc_infop->operands_count].kind
	    = ARC_OPERAND_KIND_LIMM;
	  /* It is not important to have exactly the LIMM indicator
	     here.  */
	  arc_infop->operands[arc_infop->operands_count].value = 63;
	}
      else
	{
	  arc_infop->operands[arc_infop->operands_count].value = value;
	  arc_infop->operands[arc_infop->operands_count].kind
	    = (operand->flags & ARC_OPERAND_IR
	       ? ARC_OPERAND_KIND_REG
	       : ARC_OPERAND_KIND_SHIMM);
	}
      arc_infop->operands_count ++;
    }

  /* Pretty print extra info for pc-relative operands.  */
  if (rpcl && rset)
    {
      if (info->flags & INSN_HAS_RELOC)
	/* If the instruction has a reloc associated with it, then the
	   offset field in the instruction will actually be the addend
	   for the reloc.  (We are using REL type relocs).  In such
	   cases, we can ignore the pc when computing addresses, since
	   the addend is not currently pc-relative.  */
	memaddr = 0;

      (*info->fprintf_func) (info->stream, "\t;");
      (*info->print_address_func) ((memaddr & ~3) + vpcl, info);
    }

  return insn_len;
}