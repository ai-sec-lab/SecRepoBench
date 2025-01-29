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
		unsigned int reloclength = 0;
		int leb_ret = 0;

		value = read_leb128 (start + reloc->r_offset, end, false,
			      &reloclength, &leb_ret);
		if (leb_ret != 0 || reloclength == 0 || reloclength > 8)
		  error (_("LoongArch ULEB128 field at 0x%lx contains invalid "
			   "ULEB128 value\n"),
			 (long) reloc->r_offset);

		else if (sym_index >= num_syms)
		  error (_("%s reloc contains invalid symbol index "
			   "%" PRIu64 "\n"),
			 (reloc_type == 107
			  ? "R_LARCH_ADD_ULEB128"
			  : "R_LARCH_SUB_ULEB128"),
			 sym_index);
		else
		  {
		    if (reloc_type == 107)
		      value += reloc->r_addend + symtab[sym_index].st_value;
		    else
		      value -= reloc->r_addend + symtab[sym_index].st_value;

		    /* Write uleb128 value to p.  */
		    bfd_byte *p = start + reloc->r_offset;
		    do
		      {
			bfd_byte c = value & 0x7f;
			value >>= 7;
			if (--reloclength != 0)
			  c |= 0x80;
			*p++ = c;
		      }
		    while (reloclength);
		  }

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
	      error (_("%s reloc contains invalid symbol index "
		       "%" PRIu64 "\n"), "MSP430 SYM_DIFF", sym_index);
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
		unsigned int reloclength = 0;
		int leb_ret = 0;
		switch (reloc_type)
		  {
		  case 1: /* R_MSP430_32 or R_MSP430_ABS32 */
		    reloclength = 4;
		    break;
		  case 11: /* R_MSP430_GNU_SET_ULEB128 */
		  case 22: /* R_MSP430X_GNU_SET_ULEB128 */
		    if (reloc->r_offset < (size_t) (end - start))
		      read_leb128 (start + reloc->r_offset, end, false,
				   &reloclength, &leb_ret);
		    break;
		  default:
		    reloclength = 2;
		    break;
		  }

		if (leb_ret != 0 || reloclength == 0 || reloclength > 8)
		  error (_("MSP430 ULEB128 field at %#" PRIx64
			   " contains invalid ULEB128 value\n"),
			 reloc->r_offset);
		else if (sym_index >= num_syms)
		  error (_("%s reloc contains invalid symbol index "
			   "%" PRIu64 "\n"), "MSP430", sym_index);
		else
		  {
		    value = reloc->r_addend + (symtab[sym_index].st_value
					       - saved_sym->st_value);

		    if (IN_RANGE (start, end, start + reloc->r_offset, reloclength))
		      byte_put (start + reloc->r_offset, value, reloclength);
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
	      error (_("%s reloc contains invalid symbol index "
		       "%" PRIu64 "\n"), "MN10300_SYM_DIFF", sym_index);
	    else
	      saved_sym = symtab + sym_index;
	    return true;

	  case 1: /* R_MN10300_32 */
	  case 2: /* R_MN10300_16 */
	    if (saved_sym != NULL)
	      {
		int reloclength = reloc_type == 1 ? 4 : 2;
		uint64_t value;

		if (sym_index >= num_syms)
		  error (_("%s reloc contains invalid symbol index "
			   "%" PRIu64 "\n"), "MN10300", sym_index);
		else
		  {
		    value = reloc->r_addend + (symtab[sym_index].st_value
					       - saved_sym->st_value);

		    if (IN_RANGE (start, end, start + reloc->r_offset, reloclength))
		      byte_put (start + reloc->r_offset, value, reloclength);
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
	      error (_("%s reloc contains invalid symbol index "
		       "%" PRIu64 "\n"), "RL78_SYM", sym_index);
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