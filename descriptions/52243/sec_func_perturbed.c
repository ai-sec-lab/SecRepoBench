bool
_bfd_XX_print_ce_compressed_pdata (bfd * bfdfile, void * vfile)
{
# define PDATA_ROW_SIZE	(2 * 4)
  FILE *file = (FILE *) vfile;
  bfd_byte *data = NULL;
  asection *section = bfd_get_section_by_name (bfdfile, ".pdata");
  bfd_size_type datasize = 0;
  bfd_size_type i;
  bfd_size_type start, stop;
  int onaline = PDATA_ROW_SIZE;
  struct sym_cache cache = {0, 0} ;

  if (section == NULL
      || coff_section_data (bfdfile, section) == NULL
      || pei_section_data (bfdfile, section) == NULL)
    return true;

  stop = pei_section_data (bfdfile, section)->virt_size;
  if ((stop % onaline) != 0)
    fprintf (file,
	     /* xgettext:c-format */
	     _("warning, .pdata section size (%ld) is not a multiple of %d\n"),
	     (long) stop, onaline);

  fprintf (file,
	   _("\nThe Function Table (interpreted .pdata section contents)\n"));

  fprintf (file, _("\
 vma:\t\tBegin    Prolog   Function Flags    Exception EH\n\
     \t\tAddress  Length   Length   32b exc  Handler   Data\n"));

  datasize = section->size;
  if (datasize == 0)
    return true;

  if (! bfd_malloc_and_get_section (bfdfile, section, &data))
    {
      free (data);
      return false;
    }

  start = 0;
  if (stop > datasize)
    stop = datasize;

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
}