static int
print_insn (bfd_vma memaddr, struct disassemble_info* info)
{
  int status;
  unsigned int raw_code;
  char *s = 0;
  long bytesRead = 0;
  int i = 0;
  struct xgate_opcode *opcodePTR = (struct xgate_opcode*) xgate_opcodes;
  struct decodeInfo *decodeTablePTR = 0;
  struct decodeInfo *decodePTR = 0;
  unsigned int operandRegisterBits = 0;
  signed int relAddr = 0;
  signed int operandOne = 0;
  signed int operandTwo = 0;
  bfd_byte buffer[4];
  bfd_vma absAddress;

  unsigned int operMaskReg = 0;
  /* Initialize our array of opcode masks and check them against our constant
     table.  */
  if (!initialized)
    {
      decodeTable = xmalloc (sizeof (struct decodeInfo) * xgate_num_opcodes);
      for (i = 0, decodeTablePTR = decodeTable; i < xgate_num_opcodes;
          i++, decodeTablePTR++, opcodePTR++)
        {
          unsigned int bin = 0;
          unsigned int mask = 0;
          for (s = opcodePTR->format; *s; s++)
            {
              bin <<= 1;
              mask <<= 1;
              operandRegisterBits <<= 1;
              bin |= (*s == '1');
              mask |= (*s == '0' || *s == '1');
              operandRegisterBits |= (*s == 'r');
            }
          /* Asserting will uncover inconsistencies in our table.  */
          assert ((s - opcodePTR->format) == 16 || (s - opcodePTR->format) == 32);
          assert (opcodePTR->bin_opcode == bin);

          decodeTablePTR->operMask = mask;
          decodeTablePTR->operMasksRegisterBits = operandRegisterBits;
          decodeTablePTR->opcodePTR = opcodePTR;
        }
      initialized = 1;
    }

  /* Read 16 bits.  */
  bytesRead += XGATE_TWO_BYTES;
  status = read_memory (memaddr, buffer, XGATE_TWO_BYTES, info);
  if (status == 0)
    {
      raw_code = buffer[0];
      raw_code <<= 8;
      raw_code += buffer[1];

      decodePTR = find_match (raw_code);
      if (decodePTR)
        {
          operMaskReg = decodePTR->operMasksRegisterBits;
          (*info->fprintf_func)(info->stream, "%s", decodePTR->opcodePTR->name);

          /* First we compare the shorthand format of the constraints. If we
	      still are unable to pinpoint the operands
	      we analyze the opcodes constraint string.  */
          if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_MON_R_C))
        	{
        	  (*info->fprintf_func)(info->stream, " R%x, CCR",
        		  (raw_code >> 8) & 0x7);
        	}
          else if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_MON_C_R))
            {
        	  (*info->fprintf_func)(info->stream, " CCR, R%x",
        	      (raw_code >> 8) & 0x7);
            }
          else if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_MON_R_P))
            {
        	  (*info->fprintf_func)(info->stream, " R%x, PC",
        	      (raw_code >> 8) & 0x7);
            }
          else if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_TRI))
            {
                  (*info->fprintf_func)(info->stream, " R%x, R%x, R%x",
                      (raw_code >> 8) & 0x7, (raw_code >> 5) & 0x7,
                      (raw_code >> 2) & 0x7);
            }
          else if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_IDR))
            {
                  if (raw_code & 0x01)
                    {
                      (*info->fprintf_func)(info->stream, " R%x, (R%x, R%x+)",
                          (raw_code >> 8) & 0x7, (raw_code >> 5) & 0x7,
                          (raw_code >> 2) & 0x7);
                    }
                   else if (raw_code & 0x02)
                          {
                            (*info->fprintf_func)(info->stream, " R%x, (R%x, -R%x)",
                                (raw_code >> 8) & 0x7, (raw_code >> 5) & 0x7,
                                (raw_code >> 2) & 0x7);
                          }
                   else
                     {
                       (*info->fprintf_func)(info->stream, " R%x, (R%x, R%x)",
                           (raw_code >> 8) & 0x7, (raw_code >> 5) & 0x7,
                           (raw_code >> 2) & 0x7);
                     }
            }
          else if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_DYA))
            // <MASK>
          perviousBin = raw_code;
        }
      else
        {
          (*info->fprintf_func)(info->stream,
				" unable to find opcode match #0%x", raw_code);
        }
    }
  return bytesRead;
}