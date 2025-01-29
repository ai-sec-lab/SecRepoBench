bfd_boolean needs_limm;
  const extInstruction_t *einsn, *i;
  unsigned limm = 0;
  struct arc_disassemble_info *arc_infop = info->private_data;

  /* First, try the extension instructions.  */
  if (*insn_len == 4)
    {
      einsn = arcExtMap_insn (OPCODE_32BIT_INSN (insn), insn);
      for (i = einsn; (i != NULL) && (opcode == NULL); i = i->next)
	{
	  const char *errmsg = NULL;

	  opcode = arcExtMap_genOpcode (i, isafilter, &errmsg);
	  if (opcode == NULL)
	    {
	      (*info->fprintf_func) (info->stream,
				     _("An error occured while generating the "
				       "extension instruction operations"));
	      *opcode_result = NULL;
	      return FALSE;
	    }

	  opcode = find_format_from_table (info, opcode, insn, *insn_len,
					   isafilter, &needs_limm, FALSE);
	}
    }

  /* Then, try finding the first match in the opcode table.  */
  if (opcode == NULL)
    opcode = find_format_from_table (info, arc_opcodes, insn, *insn_len,
				     isafilter, &needs_limm, TRUE);

  if (opcode != NULL && needs_limm)
    {
      bfd_byte buffer[4];
      int status;

      status = (*info->read_memory_func) (memaddr + *insn_len, buffer,
                                          4, info);
      if (status != 0)
        {
          opcode = NULL;
        }
      else
        {
          limm = ARRANGE_ENDIAN (info, buffer);
          *insn_len += 4;
        }
    }

  if (opcode != NULL)
    {
      iter->insn = insn;
      iter->limm = limm;
      iter->opcode = opcode;
      iter->opidx = opcode->operands;
    }

  *opcode_result = opcode;

  /* Update private data.  */
  arc_infop->opcode = opcode;
  arc_infop->limm = (needs_limm) ? limm : 0;