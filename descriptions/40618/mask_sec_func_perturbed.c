static int
display_debug_aranges (struct dwarf_section *section,
		       void *file ATTRIBUTE_UNUSED)
{
  unsigned char *start = section->start;
  unsigned char *end = start + section->size;

  introduce (section, false);

  /* It does not matter if this load fails,
     we test for that later on.  */
  load_debug_info (file);

  while (start < end)
    {
      unsigned char *hdrptr;
      DWARF2_Internal_ARange arange;
      unsigned char *addr_ranges;
      dwarf_vma length;
      dwarf_vma address;
      unsigned long sectionoffset;
      unsigned char address_size;
      // <MASK>
    }

  printf ("\n");

  return 1;
}