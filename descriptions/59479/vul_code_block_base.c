case EM_LOONGARCH:
      {
	switch (reloc_type)
	  {
	    /* For .uleb128 .LFE1-.LFB1, loongarch write 0 to object file
	       at assembly time.  */
	    case 107: /* R_LARCH_ADD_ULEB128.  */
	    case 108: /* R_LARCH_SUB_ULEB128.  */
	      {
		uint64_t value;
		unsigned int reloc_size = 0;
		int leb_ret = 0;

		value = read_leb128 (start + reloc->r_offset, end, false,
			      &reloc_size, &leb_ret);
		if (leb_ret != 0 || reloc_size == 0 || reloc_size > 8)
		  error (_("LoongArch ULEB128 field at 0x%lx contains invalid "
			   "ULEB128 value\n"),
			 (long) reloc->r_offset);

		if (107 == reloc_type)
		  value += (reloc->r_addend + symtab[sym_index].st_value);
		else if (108 == reloc_type)
		  value -= (reloc->r_addend + symtab[sym_index].st_value);

		/* Write uleb128 value to p.  */
		bfd_byte c;
		bfd_byte *p = start + reloc->r_offset;
		do
		  {
		    c = value & 0x7f;
		    if (reloc_size > 1)
		      c |= 0x80;
		    *(p++) = c;
		    value >>= 7;
		    reloc_size--;
		  }
		while (reloc_size);

		return true;
	      }
	  }
	break;
      }

    case EM_MSP430:
    case EM_MSP430_OLD:
      {
	static Elf_Internal_Sym * saved_sym = NULL;

	if (reloc == NULL)
	  {
	    saved_sym = NULL;
	    return true;
	  }

	switch (reloc_type)
	  {
	  case 10: /* R_MSP430_SYM_DIFF */
	  case 12: /* R_MSP430_GNU_SUB_ULEB128 */
	    if (uses_msp430x_relocs (filedata))
	      break;
	    /* Fall through.  */
	  case 21: /* R_MSP430X_SYM_DIFF */
	  case 23: /* R_MSP430X_GNU_SUB_ULEB128 */
	    /* PR 21139.  */
	    if (sym_index >= num_syms)
	      error (_("MSP430 SYM_DIFF reloc contains invalid symbol index"
		       " %" PRIu64 "\n"), sym_index);
	    else
	      saved_sym = symtab + sym_index;
	    return true;

	  case 1: /* R_MSP430_32 or R_MSP430_ABS32 */
	  case 3: /* R_MSP430_16 or R_MSP430_ABS8 */
	    goto handle_sym_diff;

	  case 5: /* R_MSP430_16_BYTE */
	  case 9: /* R_MSP430_8 */
	  case 11: /* R_MSP430_GNU_SET_ULEB128 */
	    if (uses_msp430x_relocs (filedata))
	      break;
	    goto handle_sym_diff;

	  case 2: /* R_MSP430_ABS16 */
	  case 15: /* R_MSP430X_ABS16 */
	  case 22: /* R_MSP430X_GNU_SET_ULEB128 */
	    if (! uses_msp430x_relocs (filedata))
	      break;
	    goto handle_sym_diff;

	  handle_sym_diff:
	    if (saved_sym != NULL)
	      {
		uint64_t value;
		unsigned int reloc_size = 0;
		int leb_ret = 0;
		switch (reloc_type)
		  {
		  case 1: /* R_MSP430_32 or R_MSP430_ABS32 */
		    reloc_size = 4;
		    break;
		  case 11: /* R_MSP430_GNU_SET_ULEB128 */
		  case 22: /* R_MSP430X_GNU_SET_ULEB128 */
		    if (reloc->r_offset < (size_t) (end - start))
		      read_leb128 (start + reloc->r_offset, end, false,
				   &reloc_size, &leb_ret);
		    break;
		  default:
		    reloc_size = 2;
		    break;
		  }

		if (leb_ret != 0 || reloc_size == 0 || reloc_size > 8)
		  error (_("MSP430 ULEB128 field at %#" PRIx64
			   " contains invalid ULEB128 value\n"),
			 reloc->r_offset);
		else if (sym_index >= num_syms)
		  error (_("MSP430 reloc contains invalid symbol index "
			   "%" PRIu64 "\n"),
			 sym_index);
		else
		  {
		    value = reloc->r_addend + (symtab[sym_index].st_value
					       - saved_sym->st_value);

		    if (IN_RANGE (start, end, start + reloc->r_offset, reloc_size))
		      byte_put (start + reloc->r_offset, value, reloc_size);
		    else
		      /* PR 21137 */
		      error (_("MSP430 sym diff reloc contains invalid offset: "
			       "%#" PRIx64 "\n"),
			     reloc->r_offset);
		  }

		saved_sym = NULL;
		return true;
	      }
	    break;

	  default:
	    if (saved_sym != NULL)
	      error (_("Unhandled MSP430 reloc type found after SYM_DIFF reloc\n"));
	    break;
	  }
	break;
      }

    case EM_MN10300:
    case EM_CYGNUS_MN10300:
      {
	static Elf_Internal_Sym * saved_sym = NULL;

	if (reloc == NULL)
	  {
	    saved_sym = NULL;
	    return true;
	  }

	switch (reloc_type)
	  {
	  case 34: /* R_MN10300_ALIGN */
	    return true;
	  case 33: /* R_MN10300_SYM_DIFF */
	    if (sym_index >= num_syms)
	      error (_("MN10300_SYM_DIFF reloc contains invalid symbol index "
		       "%" PRIu64 "\n"),
		     sym_index);
	    else
	      saved_sym = symtab + sym_index;
	    return true;

	  case 1: /* R_MN10300_32 */
	  case 2: /* R_MN10300_16 */
	    if (saved_sym != NULL)
	      {
		int reloc_size = reloc_type == 1 ? 4 : 2;
		uint64_t value;

		if (sym_index >= num_syms)
		  error (_("MN10300 reloc contains invalid symbol index "
			   "%" PRIu64 "\n"),
			 sym_index);
		else
		  {
		    value = reloc->r_addend + (symtab[sym_index].st_value
					       - saved_sym->st_value);

		    if (IN_RANGE (start, end, start + reloc->r_offset, reloc_size))
		      byte_put (start + reloc->r_offset, value, reloc_size);
		    else
		      error (_("MN10300 sym diff reloc contains invalid offset:"
			       " %#" PRIx64 "\n"),
			     reloc->r_offset);
		  }

		saved_sym = NULL;
		return true;
	      }
	    break;
	  default:
	    if (saved_sym != NULL)
	      error (_("Unhandled MN10300 reloc type found after SYM_DIFF reloc\n"));
	    break;
	  }
	break;
      }

    case EM_RL78:
      {
	static uint64_t saved_sym1 = 0;
	static uint64_t saved_sym2 = 0;
	static uint64_t value;

	if (reloc == NULL)
	  {
	    saved_sym1 = saved_sym2 = 0;
	    return true;
	  }

	switch (reloc_type)
	  {
	  case 0x80: /* R_RL78_SYM.  */
	    saved_sym1 = saved_sym2;
	    if (sym_index >= num_syms)
	      error (_("RL78_SYM reloc contains invalid symbol index "
		       "%" PRIu64 "\n"), sym_index);
	    else
	      {
		saved_sym2 = symtab[sym_index].st_value;
		saved_sym2 += reloc->r_addend;
	      }
	    return true;

	  case 0x83: /* R_RL78_OPsub.  */
	    value = saved_sym1 - saved_sym2;
	    saved_sym2 = saved_sym1 = 0;
	    return true;
	    break;

	  case 0x41: /* R_RL78_ABS32.  */
	    if (IN_RANGE (start, end, start + reloc->r_offset, 4))
	      byte_put (start + reloc->r_offset, value, 4);
	    else
	      error (_("RL78 sym diff reloc contains invalid offset: "
		       "%#" PRIx64 "\n"),
		     reloc->r_offset);
	    value = 0;
	    return true;

	  case 0x43: /* R_RL78_ABS16.  */
	    if (IN_RANGE (start, end, start + reloc->r_offset, 2))
	      byte_put (start + reloc->r_offset, value, 2);
	    else
	      error (_("RL78 sym diff reloc contains invalid offset: "
		       "%#" PRIx64 "\n"),
		     reloc->r_offset);
	    value = 0;
	    return true;

	  default:
	    break;
	  }
	break;
      }