
  for (i = start; i < stop; i += onaline)
    {
      bfd_vma begin_addr;
      bfd_vma other_data;
      bfd_vma prolog_length, function_length;
      int flag32bit, exception_flag;
      asection *tsection;

      if (i + PDATA_ROW_SIZE > stop)
	break;

      begin_addr = GET_PDATA_ENTRY (bfdfile, data + i     );
      other_data = GET_PDATA_ENTRY (bfdfile, data + i +  4);

      if (begin_addr == 0 && other_data == 0)
	/* We are probably into the padding of the section now.  */
	break;

      prolog_length = (other_data & 0x000000FF);
      function_length = (other_data & 0x3FFFFF00) >> 8;
      flag32bit = (int)((other_data & 0x40000000) >> 30);
      exception_flag = (int)((other_data & 0x80000000) >> 31);

      fputc (' ', file);
      bfd_fprintf_vma (bfdfile, file, i + section->vma); fputc ('\t', file);
      bfd_fprintf_vma (bfdfile, file, begin_addr); fputc (' ', file);
      bfd_fprintf_vma (bfdfile, file, prolog_length); fputc (' ', file);
      bfd_fprintf_vma (bfdfile, file, function_length); fputc (' ', file);
      fprintf (file, "%2d  %2d   ", flag32bit, exception_flag);

      /* Get the exception handler's address and the data passed from the
	 .text section. This is really the data that belongs with the .pdata
	 but got "compressed" out for the ARM and SH4 architectures.  */
      tsection = bfd_get_section_by_name (bfdfile, ".text");
      if (tsection && coff_section_data (bfdfile, tsection)
	  && pei_section_data (bfdfile, tsection))
	{
	  bfd_vma eh_off = (begin_addr - 8) - tsection->vma;
	  bfd_byte *tdata;

	  tdata = (bfd_byte *) bfd_malloc (8);
	  if (tdata)
	    {
	      if (bfd_get_section_contents (bfdfile, tsection, tdata, eh_off, 8))
		{
		  bfd_vma eh, eh_data;

		  eh = bfd_get_32 (bfdfile, tdata);
		  eh_data = bfd_get_32 (bfdfile, tdata + 4);
		  fprintf (file, "%08x  ", (unsigned int) eh);
		  fprintf (file, "%08x", (unsigned int) eh_data);
		  if (eh != 0)
		    {
		      const char *s = my_symbol_for_address (bfdfile, eh, &cache);

		      if (s)
			fprintf (file, " (%s) ", s);
		    }
		}
	      free (tdata);
	    }
	}

      fprintf (file, "\n");
    }

  free (data);

  cleanup_syms (& cache);

  return true;
#undef PDATA_ROW_SIZE