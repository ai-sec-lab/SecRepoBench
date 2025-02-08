raw_end = 0;

#define UPDATE_RAW_END(start, count, size) \
  cb_end = internal_symhdr->start + internal_symhdr->count * (size); \
  if (cb_end > raw_end) \
    raw_end = cb_end

  UPDATE_RAW_END (cbLineOffset, cbLine, sizeof (unsigned char));
  UPDATE_RAW_END (cbDnOffset, idnMax, backend->debug_swap.external_dnr_size);
  UPDATE_RAW_END (cbPdOffset, ipdMax, backend->debug_swap.external_pdr_size);
  UPDATE_RAW_END (cbSymOffset, isymMax, backend->debug_swap.external_sym_size);
  /* eraxxon@alumni.rice.edu: ioptMax refers to the size of the
     optimization symtab, not the number of entries.  */
  UPDATE_RAW_END (cbOptOffset, ioptMax, sizeof (char));
  UPDATE_RAW_END (cbAuxOffset, iauxMax, sizeof (union aux_ext));
  UPDATE_RAW_END (cbSsOffset, issMax, sizeof (char));
  UPDATE_RAW_END (cbSsExtOffset, issExtMax, sizeof (char));
  UPDATE_RAW_END (cbFdOffset, ifdMax, backend->debug_swap.external_fdr_size);
  UPDATE_RAW_END (cbRfdOffset, crfd, backend->debug_swap.external_rfd_size);
  UPDATE_RAW_END (cbExtOffset, iextMax, backend->debug_swap.external_ext_size);

#undef UPDATE_RAW_END

  raw_size = raw_end - raw_base;
  if (raw_size == 0)
    {
      ecoff_data (abfd)->sym_filepos = 0;
      return true;
    }
  pos = ecoff_data (abfd)->sym_filepos;
  pos += backend->debug_swap.external_hdr_size;
  if (bfd_seek (abfd, pos, SEEK_SET) != 0)
    return false;
  raw = _bfd_alloc_and_read (abfd, raw_size, raw_size);
  if (raw == NULL)
    return false;

  ecoff_data (abfd)->raw_syments = raw;

  /* Get pointers for the numeric offsets in the HDRR structure.  */
#define FIX(off1, off2, type)				\
  if (internal_symhdr->off1 == 0)			\
    debug->off2 = NULL;					\
  else							\
    debug->off2 = (type) ((char *) raw			\
			  + (internal_symhdr->off1	\
			     - raw_base))

  FIX (cbLineOffset, line, unsigned char *);
  FIX (cbDnOffset, external_dnr, void *);
  FIX (cbPdOffset, external_pdr, void *);
  FIX (cbSymOffset, external_sym, void *);
  FIX (cbOptOffset, external_opt, void *);
  FIX (cbAuxOffset, external_aux, union aux_ext *);
  FIX (cbSsOffset, ss, char *);
  FIX (cbSsExtOffset, ssext, char *);
  FIX (cbFdOffset, external_fdr, void *);
  FIX (cbRfdOffset, external_rfd, void *);
  FIX (cbExtOffset, external_ext, void *);
#undef FIX

  /* I don't want to always swap all the data, because it will just
     waste time and most programs will never look at it.  The only
     time the linker needs most of the debugging information swapped
     is when linking big-endian and little-endian MIPS object files
     together, which is not a common occurrence.

     We need to look at the fdr to deal with a lot of information in
     the symbols, so we swap them here.  */
  if (_bfd_mul_overflow ((unsigned long) internal_symhdr->ifdMax,
			 sizeof (struct fdr), &amt))
    {
      bfd_set_error (bfd_error_file_too_big);
      return false;
    }