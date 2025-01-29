if (stabsize == 0 || strsize == 0)
	goto out;

      if (!bfd_malloc_and_get_section (abfd, info->stabsec, &info->stabs))
	goto out;
      if (!bfd_malloc_and_get_section (abfd, info->strsec, &info->strs))
	goto out1;

      /* Stab strings ought to be nul terminated.  Ensure the last one
	 is, to prevent running off the end of the buffer.  */
      info->strs[strsize - 1] = 0;

      /* If this is a relocatable object file, we have to relocate
	 the entries in .stab.  This should always be simple 32 bit
	 relocations against symbols defined in this object file, so
	 this should be no big deal.  */
      reloc_size = bfd_get_reloc_upper_bound (abfd, info->stabsec);
      if (reloc_size < 0)
	goto out2;
      reloc_vector = (arelent **) bfd_malloc (reloc_size);
      if (reloc_vector == NULL && reloc_size != 0)
	goto out2;
      reloc_count = bfd_canonicalize_reloc (abfd, info->stabsec, reloc_vector,
					    symbols);
      if (reloc_count < 0)
	{
	out3:
	  free (reloc_vector);
	out2:
	  free (info->strs);
	  info->strs = NULL;
	out1:
	  free (info->stabs);
	  info->stabs = NULL;
	out:
	  info->stabsec = NULL;
	  return false;
	}
      if (reloc_count > 0)
	{
	  arelent **pr;

	  for (pr = reloc_vector; *pr != NULL; pr++)
	    {
	      arelent *r;
	      unsigned long val;
	      asymbol *sym;
	      bfd_size_type octets;

	      r = *pr;
	      /* Ignore R_*_NONE relocs.  */
	      if (r->howto->dst_mask == 0)
		continue;

	      octets = r->address * bfd_octets_per_byte (abfd, NULL);
	      if (r->howto->rightshift != 0
		  || bfd_get_reloc_size (r->howto) != 4
		  || r->howto->bitsize != 32
		  || r->howto->pc_relative
		  || r->howto->bitpos != 0
		  || r->howto->dst_mask != 0xffffffff
		  || octets + 4 > stabsize)
		{
		  _bfd_error_handler
		    (_("unsupported .stab relocation"));
		  bfd_set_error (bfd_error_invalid_operation);
		  goto out3;
		}

	      val = bfd_get_32 (abfd, info->stabs + octets);
	      val &= r->howto->src_mask;
	      sym = *r->sym_ptr_ptr;
	      val += sym->value + sym->section->vma + r->addend;
	      bfd_put_32 (abfd, (bfd_vma) val, info->stabs + octets);
	    }
	}

      free (reloc_vector);

      /* First time through this function, build a table matching
	 function VM addresses to stabs, then sort based on starting
	 VM address.  Do this in two passes: once to count how many
	 table entries we'll need, and a second to actually build the
	 table.  */

      info->indextablesize = 0;
      nul_fun = NULL;
      for (stab = info->stabs; stab < info->stabs + stabsize; stab += STABSIZE)
	{
	  if (stab[TYPEOFF] == (bfd_byte) N_SO)
	    {
	      /* if we did not see a function def, leave space for one.  */
	      if (nul_fun != NULL)
		++info->indextablesize;

	      /* N_SO with null name indicates EOF */
	      if (bfd_get_32 (abfd, stab + STRDXOFF) == 0)
		nul_fun = NULL;
	      else
		{
		  nul_fun = stab;

		  /* two N_SO's in a row is a filename and directory. Skip */
		  if (stab + STABSIZE + TYPEOFF < info->stabs + stabsize
		      && *(stab + STABSIZE + TYPEOFF) == (bfd_byte) N_SO)
		    stab += STABSIZE;
		}
	    }
	  else if (stab[TYPEOFF] == (bfd_byte) N_FUN
		   && bfd_get_32 (abfd, stab + STRDXOFF) != 0)
	    {
	      nul_fun = NULL;
	      ++info->indextablesize;
	    }
	}

      if (nul_fun != NULL)
	++info->indextablesize;

      if (info->indextablesize == 0)
	{
	  free (info->strs);
	  info->strs = NULL;
	  free (info->stabs);
	  info->stabs = NULL;
	  info->stabsec = NULL;
	  return true;
	}
      ++info->indextablesize;

      amt = info->indextablesize;
      amt *= sizeof (struct indexentry);
      info->indextable = (struct indexentry *) bfd_malloc (amt);
      if (info->indextable == NULL)
	goto out3;

      file_name = NULL;
      directory_name = NULL;
      nul_fun = NULL;
      stroff = 0;

      for (i = 0, stab = info->stabs, nul_str = str = info->strs;
	   i < info->indextablesize && stab < info->stabs + stabsize;
	   stab += STABSIZE)
	{
	  switch (stab[TYPEOFF])
	    {
	    case 0:
	      /* This is the first entry in a compilation unit.  */
	      if ((bfd_size_type) ((info->strs + strsize) - str) < stroff)
		break;
	      str += stroff;
	      stroff = bfd_get_32 (abfd, stab + VALOFF);
	      break;

	    case N_SO:
	      /* The main file name.  */

	      /* The following code creates a new indextable entry with
		 a NULL function name if there were no N_FUNs in a file.
		 Note that a N_SO without a file name is an EOF and
		 there could be 2 N_SO following it with the new filename
		 and directory.  */
	      if (nul_fun != NULL)
		{
		  info->indextable[i].val = bfd_get_32 (abfd, nul_fun + VALOFF);
		  info->indextable[i].stab = nul_fun;
		  info->indextable[i].str = nul_str;
		  info->indextable[i].directory_name = directory_name;
		  info->indextable[i].file_name = file_name;
		  info->indextable[i].function_name = NULL;
		  info->indextable[i].idx = i;
		  ++i;
		}

	      directory_name = NULL;
	      file_name = (char *) str + bfd_get_32 (abfd, stab + STRDXOFF);
	      if (file_name == (char *) str)
		{
		  file_name = NULL;
		  nul_fun = NULL;
		}
	      else
		{
		  nul_fun = stab;
		  nul_str = str;
		  if (file_name >= (char *) info->strs + strsize
		      || file_name < (char *) str)
		    file_name = NULL;
		  if (stab + STABSIZE + TYPEOFF < info->stabs + stabsize
		      && *(stab + STABSIZE + TYPEOFF) == (bfd_byte) N_SO)
		    {
		      /* Two consecutive N_SOs are a directory and a
			 file name.  */
		      stab += STABSIZE;
		      directory_name = file_name;
		      file_name = ((char *) str
				   + bfd_get_32 (abfd, stab + STRDXOFF));
		      if (file_name >= (char *) info->strs + strsize
			  || file_name < (char *) str)
			file_name = NULL;
		    }
		}
	      break;

	    case N_SOL:
	      /* The name of an include file.  */
	      file_name = (char *) str + bfd_get_32 (abfd, stab + STRDXOFF);
	      /* PR 17512: file: 0c680a1f.  */
	      /* PR 17512: file: 5da8aec4.  */
	      if (file_name >= (char *) info->strs + strsize
		  || file_name < (char *) str)
		file_name = NULL;
	      break;

	    case N_FUN:
	      /* A function name.  */
	      function_name = (char *) str + bfd_get_32 (abfd, stab + STRDXOFF);
	      if (function_name == (char *) str)
		continue;
	      if (function_name >= (char *) info->strs + strsize
		  || function_name < (char *) str)
		function_name = NULL;

	      nul_fun = NULL;
	      info->indextable[i].val = bfd_get_32 (abfd, stab + VALOFF);
	      info->indextable[i].stab = stab;
	      info->indextable[i].str = str;
	      info->indextable[i].directory_name = directory_name;
	      info->indextable[i].file_name = file_name;
	      info->indextable[i].function_name = function_name;
	      info->indextable[i].idx = i;
	      ++i;
	      break;
	    }
	}

      if (nul_fun != NULL)
	{
	  info->indextable[i].val = bfd_get_32 (abfd, nul_fun + VALOFF);
	  info->indextable[i].stab = nul_fun;
	  info->indextable[i].str = nul_str;
	  info->indextable[i].directory_name = directory_name;
	  info->indextable[i].file_name = file_name;
	  info->indextable[i].function_name = NULL;
	  info->indextable[i].idx = i;
	  ++i;
	}

      info->indextable[i].val = (bfd_vma) -1;
      info->indextable[i].stab = info->stabs + stabsize;
      info->indextable[i].str = str;
      info->indextable[i].directory_name = NULL;
      info->indextable[i].file_name = NULL;
      info->indextable[i].function_name = NULL;
      info->indextable[i].idx = i;
      ++i;

      info->indextablesize = i;
      qsort (info->indextable, (size_t) i, sizeof (struct indexentry),
	     cmpindexentry);