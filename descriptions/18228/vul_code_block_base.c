{
        	  operandOne = ripBits (&operMaskReg, 3, opcodePTR, raw_code);
        	  operandTwo = ripBits (&operMaskReg, 3, opcodePTR, raw_code);
        	 ( *info->fprintf_func)(info->stream, " R%x, R%x", operandOne,
        	      operandTwo);
            }
          else if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_IDO5))
            {
        	  (*info->fprintf_func)(info->stream, " R%x, (R%x, #0x%x)",
        	      (raw_code >> 8) & 0x7, (raw_code >> 5) & 0x7, raw_code & 0x1f);
            }
          else if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_MON))
            {
        	  operandOne = ripBits (&operMaskReg, 3, decodePTR->opcodePTR,
        	     raw_code);
        	 (*info->fprintf_func)(info->stream, " R%x", operandOne);
            }
          else if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_REL9))
            {
              /* If address is negative handle it accordingly.  */
              if (raw_code & XGATE_NINE_SIGNBIT)
                {
                  relAddr = XGATE_NINE_BITS >> 1; /* Clip sign bit.  */
                  relAddr = ~relAddr; /* Make signed.  */
                  relAddr |= (raw_code & 0xFF) + 1; /* Apply our value.  */
                  relAddr <<= 1; /* Multiply by two as per processor docs.  */
                }
              else
                {
                  relAddr = raw_code & 0xff;
                  relAddr = (relAddr << 1) + 2;
                }
             (*info->fprintf_func)(info->stream, " *%d", relAddr);
             (*info->fprintf_func)(info->stream, "  Abs* 0x");
             (*info->print_address_func)(memaddr + relAddr, info);
           }
          else if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_REL10))
            {
              /* If address is negative handle it accordingly.  */
              if (raw_code & XGATE_TEN_SIGNBIT)
                {
                  relAddr = XGATE_TEN_BITS >> 1; /* Clip sign bit.  */
                  relAddr = ~relAddr; /* Make signed.  */
                  relAddr |= (raw_code & 0x1FF) + 1; /* Apply our value.  */
                  relAddr <<= 1; /* Multiply by two as per processor docs.  */
                }
              else
                {
                  relAddr = raw_code & 0x1FF;
                  relAddr = (relAddr << 1) + 2;
                }
              (*info->fprintf_func)(info->stream, " *%d", relAddr);
              (*info->fprintf_func)(info->stream, "  Abs* 0x");
              (*info->print_address_func)(memaddr + relAddr, info);
            }
          else if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_IMM4))
            {
              (*info->fprintf_func)(info->stream, " R%x, #0x%02x",
              (raw_code >> 8) & 0x7, (raw_code >> 4) & 0xF);
            }
          else if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_IMM8))
            {
              if (macro_search (decodePTR->opcodePTR->name, previousOpName) &&
                 previousOpName[0])
               {
                 absAddress = (0xFF & raw_code) << 8;
                 absAddress |= perviousBin & 0xFF;
                 (*info->fprintf_func)(info->stream, " R%x, #0x%02x Abs* 0x",
                     (raw_code >> 8) & 0x7, raw_code & 0xff);
                 (*info->print_address_func)(absAddress, info);
                 previousOpName[0] = 0;
               }
              else
               {
                 strcpy (previousOpName, decodePTR->opcodePTR->name);
                 (*info->fprintf_func)(info->stream, " R%x, #0x%02x",
                     (raw_code >> 8) & 0x7, raw_code & 0xff);
               }
            }
          else if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_IMM3))
            {
        	  (*info->fprintf_func)(info->stream, " #0x%x",
        	     (raw_code >> 8) & 0x7);
            }
          else if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_INH))
            {
        	  //
            }
          else
            {
              (*info->fprintf_func)(info->stream, " unhandled mode %s",
                opcodePTR->constraints);
            }