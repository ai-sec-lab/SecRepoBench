	    old_flags = bfd_getl16 (egst->header.flags);

	    if (nameoffset >= gsd_size)
	      goto too_small;
	    entry = add_symbol (abfd, &egst->namlng, gsd_size - nameoffset);
	    if (entry == NULL)
	      return FALSE;
