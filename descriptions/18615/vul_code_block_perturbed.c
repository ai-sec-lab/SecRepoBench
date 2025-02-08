char operand[2][13] =
  {
    {0},
    {0}
  };
  unsigned long address;
  int print_label = 0;

  if (insn->tm == NULL)
    return 0;
  /* Get the operands for 24-bit immediate jumps.  */
  if (insn->tm->operand_types[0] & Imm24)
    {
      address = instructionword & 0x00FFFFFF;
      sprintf (operand[0], "0x%lX", address);
      print_label = 1;
    }
  /* Get the operand for the trap instruction.  */
  else if (insn->tm->operand_types[0] & IVector)
    {
      address = instructionword & 0x0000001F;
      sprintf (operand[0], "0x%lX", address);
    }
  else
    {
      address = instructionword & 0x0000FFFF;
      /* Get the operands for the DB instructions.  */
      if (insn->tm->operands == 2)
	{
	  get_register_operand (((instructionword & 0x01C00000) >> 22) + REG_AR0, operand[0]);
	  if (instructionword & PCRel)
	    {
	      sprintf (operand[1], "%d", (short) address);
	      print_label = 1;
	    }
	  else
	    get_register_operand (instructionword & 0x0000001F, operand[1]);
	}
      /* Get the operands for the standard branches.  */
      else if (insn->tm->operands == 1)
	{
	  if (instructionword & PCRel)
	    {
	      address = (short) address;
	      sprintf (operand[0], "%ld", address);
	      print_label = 1;
	    }
	  else
	    get_register_operand (instructionword & 0x0000001F, operand[0]);
	}
    }
  info->fprintf_func (info->stream, "   %s %s%c%s", insn->tm->name,
		      operand[0][0] ? operand[0] : "",
		      operand[1][0] ? ',' : ' ',
		      operand[1][0] ? operand[1] : "");
  /* Print destination of branch in relation to current symbol.  */
  if (print_label && info->symbols)
    {
      asymbol *sym = *info->symbols;

      if ((insn->tm->opcode_modifier == PCRel) && (instructionword & PCRel))
	{
	  address = (_pc + 1 + (short) address) - ((sym->section->vma + sym->value) / 4);
	  /* Check for delayed instruction, if so adjust destination.  */
	  if (instructionword & 0x00200000)
	    address += 2;
	}
      else
	{
	  address -= ((sym->section->vma + sym->value) / 4);
	}
      if (address == 0)
	info->fprintf_func (info->stream, " <%s>", sym->name);
      else
	info->fprintf_func (info->stream, " <%s %c %lu>", sym->name,
			    ((short) address < 0) ? '-' : '+',
			    address);
    }
  return 1;