static bool
som_slurp_reloc_table (bfd *abfd,
		       asection *section,
		       asymbol **symbols,
		       bool just_count)
{
  unsigned char *external_relocs;
  unsigned int fixup_stream_size;
  arelent *internal_relocs;
  unsigned int num_relocs;
  size_t amt;

  fixup_stream_size = som_section_data (section)->reloc_size;
  /* If there were no relocations, then there is nothing to do.  */
  if (section->reloc_count == 0)
    return true;

  /* If reloc_count is -1, then the relocation stream has not been
     parsed.  We must do so now to know how many relocations exist.  */
  // <MASK>

  /* We're done with the external relocations.  Free them.  */
  free (external_relocs);
  som_section_data (section)->reloc_stream = NULL;

  /* Save our results and return success.  */
  section->relocation = internal_relocs;
  return true;
}