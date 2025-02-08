static long
elf32_arm_get_synthetic_symtab (bfd *abfd,
			       long symcount ATTRIBUTE_UNUSED,
			       asymbol **syms ATTRIBUTE_UNUSED,
			       long dynamicsymbolcount,
			       asymbol **dynsyms,
			       asymbol **ret)
{
  asection *relplt;
  asymbol *s;
  arelent *p;
  long count, i, n;
  size_t size;
  Elf_Internal_Shdr *hdr;
  char *names;
  asection *plt;
  bfd_vma offset;
  bfd_byte *data;

  *ret = NULL;

  if ((abfd->flags & (DYNAMIC | EXEC_P)) == 0)
    return 0;

  if (dynamicsymbolcount <= 0)
    return 0;

  relplt = bfd_get_section_by_name (abfd, ".rel.plt");
  if (relplt == NULL)
    return 0;

  hdr = &elf_section_data (relplt)->this_hdr;
  if (hdr->sh_link != elf_dynsymtab (abfd)
      || (hdr->sh_type != SHT_REL && hdr->sh_type != SHT_RELA))
    return 0;

  plt = bfd_get_section_by_name (abfd, ".plt");
  if (plt == NULL)
    return 0;

  if (!elf32_arm_size_info.slurp_reloc_table (abfd, relplt, dynsyms, true))
    return -1;

  data = NULL;
  if (!bfd_get_full_section_contents (abfd, plt, &data))
    return -1;

  count = NUM_SHDR_ENTRIES (hdr);
  size = count * sizeof (asymbol);
  p = relplt->relocation;
  for (i = 0; i < count; i++, p += elf32_arm_size_info.int_rels_per_ext_rel)
    {
      size += strlen ((*p->sym_ptr_ptr)->name) + sizeof ("@plt");
      if (p->addend != 0)
	size += sizeof ("+0x") - 1 + 8;
    }

  offset = elf32_arm_plt0_size (abfd, data, plt->size);
  if (offset == (bfd_vma) -1
      || (s = *ret = (asymbol *) bfd_malloc (size)) == NULL)
    {
      free (data);
      return -1;
    }

  names = (char *) (s + count);
  p = relplt->relocation;
  n = 0;
  for (i = 0; i < count; i++, p += elf32_arm_size_info.int_rels_per_ext_rel)
    {
      size_t len;

      bfd_vma plt_size = elf32_arm_plt_size (abfd, data, offset, plt->size);
      if (plt_size == (bfd_vma) -1)
	break;

      *s = **p->sym_ptr_ptr;
      /* Undefined syms won't have BSF_LOCAL or BSF_GLOBAL set.  Since
	 we are defining a symbol, ensure one of them is set.  */
      if ((s->flags & BSF_LOCAL) == 0)
	s->flags |= BSF_GLOBAL;
      s->flags |= BSF_SYNTHETIC;
      s->section = plt;
      s->value = offset;
      s->name = names;
      s->udata.p = NULL;
      len = strlen ((*p->sym_ptr_ptr)->name);
      memcpy (names, (*p->sym_ptr_ptr)->name, len);
      names += len;
      if (p->addend != 0)
	{
	  char buf[30], *a;

	  memcpy (names, "+0x", sizeof ("+0x") - 1);
	  names += sizeof ("+0x") - 1;
	  bfd_sprintf_vma (abfd, buf, p->addend);
	  for (a = buf; *a == '0'; ++a)
	    ;
	  len = strlen (a);
	  memcpy (names, a, len);
	  names += len;
	}
      memcpy (names, "@plt", sizeof ("@plt"));
      names += sizeof ("@plt");
      ++s, ++n;
      offset += plt_size;
    }

  free (data);
  return n;
}