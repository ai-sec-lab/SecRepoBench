long reloc_count;
	      arelent **relocs;

	      relocs = (arelent **) xmalloc (reloc_size);

	      reloc_count = bfd_canonicalize_reloc (abfd, sec, relocs, NULL);
	      if (reloc_count <= 0)
		free (relocs);
	      else
		{
		  section->reloc_info = relocs;
		  section->num_relocs = reloc_count;
		}