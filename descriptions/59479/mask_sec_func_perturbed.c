static bool
target_specific_reloc_handling (Filedata *filedata,
				Elf_Internal_Rela *reloc,
				unsigned char *start,
				unsigned char *end,
				Elf_Internal_Sym *symtab,
				uint64_t num_syms)
{
  unsigned int reloc_type = 0;
  uint64_t sym_index = 0;

  if (reloc)
    {
      reloc_type = get_reloc_type (filedata, reloc->r_info);
      sym_index = get_reloc_symindex (reloc->r_info);
    }

  switch (filedata->file_header.e_machine)
    {
    // <MASK>
    }

  return false;
}