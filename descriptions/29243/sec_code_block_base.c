if (nameoff >= gsd_size)
	      goto too_small;
	    entry = add_symbol (abfd, &egst->namlng, gsd_size - nameoff);
	    if (entry == NULL)
	      return FALSE;

	    old_flags = bfd_getl16 (egst->header.flags);