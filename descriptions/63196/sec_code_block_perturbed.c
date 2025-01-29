for (arelent **relp = reloc_vector; *relp != NULL; relp++)
    {
      arelent *rel;
      bfd_reloc_status_type r;
      char *err;

      rel = *relp;
      r = bfd_reloc_ok;
      switch (rel->howto->type)
	{
	case ALPHA_R_IGNORE:
	  rel->address += input_section->output_offset;
	  break;

	case ALPHA_R_REFLONG:
	case ALPHA_R_REFQUAD:
	case ALPHA_R_BRADDR:
	case ALPHA_R_HINT:
	case ALPHA_R_SREL16:
	case ALPHA_R_SREL32:
	case ALPHA_R_SREL64:
	  if (relocatable
	      && ((*rel->sym_ptr_ptr)->flags & BSF_SECTION_SYM) == 0)
	    {
	      rel->address += input_section->output_offset;
	      break;
	    }
	  r = bfd_perform_relocation (input_bfd, rel, data, input_section,
				      output_bfd, &err);
	  break;

	case ALPHA_R_GPREL32:
	  /* This relocation is used in a switch table.  It is a 32
	     bit offset from the current GP value.  We must adjust it
	     by the different between the original GP value and the
	     current GP value.  The original GP value is stored in the
	     addend.  We adjust the addend and let
	     bfd_perform_relocation finish the job.  */
	  rel->addend -= gp;
	  r = bfd_perform_relocation (input_bfd, rel, data, input_section,
				      output_bfd, &err);
	  if (r == bfd_reloc_ok && gpnotdefined)
	    {
	      r = bfd_reloc_dangerous;
	      err = (char *) _("GP relative relocation used when GP not defined");
	    }
	  break;

	case ALPHA_R_LITERAL:
	  /* This is a reference to a literal value, generally
	     (always?) in the .lita section.  This is a 16 bit GP
	     relative relocation.  Sometimes the subsequent reloc is a
	     LITUSE reloc, which indicates how this reloc is used.
	     This sometimes permits rewriting the two instructions
	     referred to by the LITERAL and the LITUSE into different
	     instructions which do not refer to .lita.  This can save
	     a memory reference, and permits removing a value from
	     .lita thus saving GP relative space.

	     We do not these optimizations.  To do them we would need
	     to arrange to link the .lita section first, so that by
	     the time we got here we would know the final values to
	     use.  This would not be particularly difficult, but it is
	     not currently implemented.  */

	  {
	    unsigned long insn;

	    /* I believe that the LITERAL reloc will only apply to a
	       ldq or ldl instruction, so check my assumption.  */
	    insn = bfd_get_32 (input_bfd, data + rel->address);
	    BFD_ASSERT (((insn >> 26) & 0x3f) == 0x29
			|| ((insn >> 26) & 0x3f) == 0x28);

	    rel->addend -= gp;
	    r = bfd_perform_relocation (input_bfd, rel, data, input_section,
					output_bfd, &err);
	    if (r == bfd_reloc_ok && gpnotdefined)
	      {
		r = bfd_reloc_dangerous;
		err =
		  (char *) _("GP relative relocation used when GP not defined");
	      }
	  }
	  break;

	case ALPHA_R_LITUSE:
	  /* See ALPHA_R_LITERAL above for the uses of this reloc.  It
	     does not cause anything to happen, itself.  */
	  rel->address += input_section->output_offset;
	  break;

	case ALPHA_R_GPDISP:
	  /* This marks the ldah of an ldah/lda pair which loads the
	     gp register with the difference of the gp value and the
	     current location.  The second of the pair is r_size bytes
	     ahead; it used to be marked with an ALPHA_R_IGNORE reloc,
	     but that no longer happens in OSF/1 3.2.  */
	  {
	    unsigned long insn1, insn2;
	    bfd_vma addend;

	    /* Get the two instructions.  */
	    insn1 = bfd_get_32 (input_bfd, data + rel->address);
	    insn2 = bfd_get_32 (input_bfd, data + rel->address + rel->addend);

	    BFD_ASSERT (((insn1 >> 26) & 0x3f) == 0x09); /* ldah */
	    BFD_ASSERT (((insn2 >> 26) & 0x3f) == 0x08); /* lda */

	    /* Get the existing addend.  We must account for the sign
	       extension done by lda and ldah.  */
	    addend = ((insn1 & 0xffff) << 16) + (insn2 & 0xffff);
	    if (insn1 & 0x8000)
	      {
		addend -= 0x80000000;
		addend -= 0x80000000;
	      }
	    if (insn2 & 0x8000)
	      addend -= 0x10000;

	    /* The existing addend includes the different between the
	       gp of the input BFD and the address in the input BFD.
	       Subtract this out.  */
	    addend -= (ecoff_data (input_bfd)->gp
		       - (input_section->vma + rel->address));

	    /* Now add in the final gp value, and subtract out the
	       final address.  */
	    addend += (gp
		       - (input_section->output_section->vma
			  + input_section->output_offset
			  + rel->address));

	    /* Change the instructions, accounting for the sign
	       extension, and write them out.  */
	    if (addend & 0x8000)
	      addend += 0x10000;
	    insn1 = (insn1 & 0xffff0000) | ((addend >> 16) & 0xffff);
	    insn2 = (insn2 & 0xffff0000) | (addend & 0xffff);

	    bfd_put_32 (input_bfd, (bfd_vma) insn1, data + rel->address);
	    bfd_put_32 (input_bfd, (bfd_vma) insn2,
			data + rel->address + rel->addend);

	    rel->address += input_section->output_offset;
	  }
	  break;

	case ALPHA_R_OP_PUSH:
	  /* Push a value on the reloc evaluation stack.  */
	  {
	    asymbol *symbol;
	    bfd_vma relocation;

	    if (relocatable)
	      {
		rel->address += input_section->output_offset;
		break;
	      }

	    /* Figure out the relocation of this symbol.  */
	    symbol = *rel->sym_ptr_ptr;

	    if (bfd_is_und_section (symbol->section))
	      r = bfd_reloc_undefined;

	    if (bfd_is_com_section (symbol->section))
	      relocation = 0;
	    else
	      relocation = symbol->value;
	    relocation += symbol->section->output_section->vma;
	    relocation += symbol->section->output_offset;
	    relocation += rel->addend;

	    if (tos >= RELOC_STACKSIZE)
	      abort ();

	    stack[tos++] = relocation;
	  }
	  break;

	case ALPHA_R_OP_STORE:
	  /* Store a value from the reloc stack into a bitfield.  */
	  {
	    bfd_vma val;
	    int offset, size;

	    if (relocatable)
	      {
		rel->address += input_section->output_offset;
		break;
	      }

	    if (tos == 0)
	      abort ();

	    /* The offset and size for this reloc are encoded into the
	       addend field by alpha_adjust_reloc_in.  */
	    offset = (rel->addend >> 8) & 0xff;
	    size = rel->addend & 0xff;

	    val = bfd_get_64 (abfd, data + rel->address);
	    val &=~ (((1 << size) - 1) << offset);
	    val |= (stack[--tos] & ((1 << size) - 1)) << offset;
	    bfd_put_64 (abfd, val, data + rel->address);
	  }
	  break;

	case ALPHA_R_OP_PSUB:
	  /* Subtract a value from the top of the stack.  */
	  {
	    asymbol *symbol;
	    bfd_vma relocation;

	    if (relocatable)
	      {
		rel->address += input_section->output_offset;
		break;
	      }

	    /* Figure out the relocation of this symbol.  */
	    symbol = *rel->sym_ptr_ptr;

	    if (bfd_is_und_section (symbol->section))
	      r = bfd_reloc_undefined;

	    if (bfd_is_com_section (symbol->section))
	      relocation = 0;
	    else
	      relocation = symbol->value;
	    relocation += symbol->section->output_section->vma;
	    relocation += symbol->section->output_offset;
	    relocation += rel->addend;

	    if (tos == 0)
	      abort ();

	    stack[tos - 1] -= relocation;
	  }
	  break;

	case ALPHA_R_OP_PRSHIFT:
	  /* Shift the value on the top of the stack.  */
	  {
	    asymbol *symbol;
	    bfd_vma relocation;

	    if (relocatable)
	      {
		rel->address += input_section->output_offset;
		break;
	      }

	    /* Figure out the relocation of this symbol.  */
	    symbol = *rel->sym_ptr_ptr;

	    if (bfd_is_und_section (symbol->section))
	      r = bfd_reloc_undefined;

	    if (bfd_is_com_section (symbol->section))
	      relocation = 0;
	    else
	      relocation = symbol->value;
	    relocation += symbol->section->output_section->vma;
	    relocation += symbol->section->output_offset;
	    relocation += rel->addend;

	    if (tos == 0)
	      abort ();

	    stack[tos - 1] >>= relocation;
	  }
	  break;

	case ALPHA_R_GPVALUE:
	  /* I really don't know if this does the right thing.  */
	  gp = rel->addend;
	  gpnotdefined = false;
	  break;

	default:
	  abort ();
	}

      if (relocatable)
	{
	  asection *os = input_section->output_section;

	  /* A partial link, so keep the relocs.  */
	  os->orelocation[os->reloc_count] = rel;
	  os->reloc_count++;
	}

      if (r != bfd_reloc_ok)
	{
	  switch (r)
	    {
	    case bfd_reloc_undefined:
	      (*link_info->callbacks->undefined_symbol)
		(link_info, bfd_asymbol_name (*rel->sym_ptr_ptr),
		 input_bfd, input_section, rel->address, true);
	      break;
	    case bfd_reloc_dangerous:
	      (*link_info->callbacks->reloc_dangerous)
		(link_info, err, input_bfd, input_section, rel->address);
	      break;
	    case bfd_reloc_overflow:
	      (*link_info->callbacks->reloc_overflow)
		(link_info, NULL, bfd_asymbol_name (*rel->sym_ptr_ptr),
		 rel->howto->name, rel->addend, input_bfd,
		 input_section, rel->address);
	      break;
	    case bfd_reloc_outofrange:
	    default:
	      abort ();
	      break;
	    }
	}
    }

  if (tos != 0)
    abort ();

 successful_return:
  free (reloc_vector);
  return data;

 error_return:
  free (reloc_vector);
  if (orig_data == NULL)
    free (data);
  return NULL;