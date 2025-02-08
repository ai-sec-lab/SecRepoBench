bool
_bfd_XX_print_ce_compressed_pdata (bfd * abfd, void * vfile)
{
# define PDATA_ROW_SIZE	(2 * 4)
  FILE *file = (FILE *) vfile;
  bfd_byte *data = NULL;
  asection *section = bfd_get_section_by_name (abfd, ".pdata");
  bfd_size_type datasize = 0;
  bfd_size_type i;
  bfd_size_type start, stop;
  int onaline = PDATA_ROW_SIZE;
  struct sym_cache cache = {0, 0} ;

  if (section == NULL
      || coff_section_data (abfd, section) == NULL
      || pei_section_data (abfd, section) == NULL)
    return true;

  stop = pei_section_data (abfd, section)->virt_size;
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

  if (! bfd_malloc_and_get_section (abfd, section, &data))
    {
      free (data);
      return false;
    }

  start = 0;
  // <MASK>
}