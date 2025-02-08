bool
_bfd_ecoff_slurp_symbolic_info (bfd *abfd,
				asection *ignore ATTRIBUTE_UNUSED,
				struct ecoff_debug_info *debug)
{
  const struct ecoff_backend_data * const backend = ecoff_backend (abfd);
  HDRR *internal_symhdr;
  bfd_size_type raw_base;
  bfd_size_type raw_size;
  void * raw;
  bfd_size_type external_fdr_size;
  char *fraw_src;
  char *fraw_end;
  struct fdr *fdr_ptr;
  bfd_size_type raw_end;
  bfd_size_type cb_end;
  file_ptr pos;
  size_t amt;

  BFD_ASSERT (debug == &ecoff_data (abfd)->debug_info);

  /* Check whether we've already gotten it, and whether there's any to
     get.  */
  if (ecoff_data (abfd)->raw_syments != NULL)
    return true;
  if (ecoff_data (abfd)->sym_filepos == 0)
    {
      abfd->symcount = 0;
      return true;
    }

  if (! ecoff_slurp_symbolic_header (abfd))
    return false;

  internal_symhdr = &debug->symbolic_header;

  /* Read all the symbolic information at once.  */
  raw_base = (ecoff_data (abfd)->sym_filepos
	      + backend->debug_swap.external_hdr_size);

  /* Alpha ecoff makes the determination of raw_size difficult. It has
     an undocumented debug data section between the symhdr and the first
     documented section. And the ordering of the sections varies between
     statically and dynamically linked executables.
     If bfd supports SEEK_END someday, this code could be simplified.  */
  raw_end = raw_base;

#define UPDATE_RAW_END(start, count, size) \
  do									\
    if (internal_symhdr->count != 0)					\
      {									\
	if (internal_symhdr->start < raw_base)				\
	  goto err;							\
	if (_bfd_mul_overflow ((unsigned long) internal_symhdr->count,	\
			       (size), &amt))				\
	  goto err;							\
	cb_end = internal_symhdr->start + amt;				\
	if (cb_end < internal_symhdr->start)				\
	  goto err;							\
	if (cb_end > raw_end)						\
	  raw_end = cb_end;						\
      }									\
  while (0)

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
#define FIX(start, count, ptr, type) \
  if (internal_symhdr->start == 0 || internal_symhdr->count == 0)	\
    debug->ptr = NULL;							\
  else									\
    debug->ptr = (type) ((char *) raw					\
			 + (internal_symhdr->start - raw_base))

  FIX (cbLineOffset, cbLine, line, unsigned char *);
  FIX (cbDnOffset, idnMax, external_dnr, void *);
  FIX (cbPdOffset, ipdMax, external_pdr, void *);
  FIX (cbSymOffset, isymMax, external_sym, void *);
  FIX (cbOptOffset, ioptMax, external_opt, void *);
  FIX (cbAuxOffset, iauxMax, external_aux, union aux_ext *);
  FIX (cbSsOffset, issMax, ss, char *);
  FIX (cbSsExtOffset, issExtMax, ssext, char *);
  FIX (cbFdOffset, ifdMax, external_fdr, void *);
  FIX (cbRfdOffset, crfd, external_rfd, void *);
  FIX (cbExtOffset, iextMax, external_ext, void *);
#undef FIX

  /* Ensure string sections are zero terminated.  */
  // <MASK>
}