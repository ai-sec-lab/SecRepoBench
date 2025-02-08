static bfd_byte *
alpha_ecoff_get_relocated_section_contents (bfd *abfd,
					    struct bfd_link_info *link_info,
					    struct bfd_link_order *link_order,
					    bfd_byte *data,
					    bool relocatable,
					    asymbol **symbols)
{
  bfd *input_bfd = link_order->u.indirect.section->owner;
  asection *input_section = link_order->u.indirect.section;
  long reloc_size;
  arelent **reloc_vector;
  long reloc_count;
  bfd *output_bfd = relocatable ? abfd : (bfd *) NULL;
  bfd_vma gp;
  bool gpnotdefined;
  bfd_vma stack[RELOC_STACKSIZE];
  int tos = 0;

  reloc_size = bfd_get_reloc_upper_bound (input_bfd, input_section);
  if (reloc_size < 0)
    return NULL;

  bfd_byte *orig_data = data;
  if (!bfd_get_full_section_contents (input_bfd, input_section, &data))
    return NULL;

  if (data == NULL)
    return NULL;

  if (reloc_size == 0)
    return data;

  reloc_vector = (arelent **) bfd_malloc (reloc_size);
  if (reloc_vector == NULL)
    goto error_return;

  reloc_count = bfd_canonicalize_reloc (input_bfd, input_section,
					reloc_vector, symbols);
  if (reloc_count < 0)
    goto error_return;
  if (reloc_count == 0)
    goto successful_return;

  /* Get the GP value for the output BFD.  */
  gpnotdefined = false;
  gp = _bfd_get_gp_value (abfd);
  if (gp == 0)
    {
      if (relocatable)
	{
	  asection *sec;
	  bfd_vma lo;

	  /* Make up a value.  */
	  lo = (bfd_vma) -1;
	  for (sec = abfd->sections; sec != NULL; sec = sec->next)
	    {
	      if (sec->vma < lo
		  && (strcmp (sec->name, ".sbss") == 0
		      || strcmp (sec->name, ".sdata") == 0
		      || strcmp (sec->name, ".lit4") == 0
		      || strcmp (sec->name, ".lit8") == 0
		      || strcmp (sec->name, ".lita") == 0))
		lo = sec->vma;
	    }
	  gp = lo + 0x8000;
	  _bfd_set_gp_value (abfd, gp);
	}
      else
	{
	  struct bfd_link_hash_entry *h;

	  h = bfd_link_hash_lookup (link_info->hash, "_gp", false, false,
				    true);
	  if (h == (struct bfd_link_hash_entry *) NULL
	      || h->type != bfd_link_hash_defined)
	    gpnotdefined = true;
	  else
	    {
	      gp = (h->u.def.value
		    + h->u.def.section->output_section->vma
		    + h->u.def.section->output_offset);
	      _bfd_set_gp_value (abfd, gp);
	    }
	}
    }

  // <MASK>
}