bool
_bfd_ecoff_slurp_symbolic_info (bfd *abfd,
				asection *ignore UNUSEDSECTION,
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
  // <MASK>
  debug->fdr = (FDR *) bfd_alloc (abfd, amt);
  if (debug->fdr == NULL)
    return false;
  external_fdr_size = backend->debug_swap.external_fdr_size;
  fdr_ptr = debug->fdr;
  fraw_src = (char *) debug->external_fdr;
  /* PR 17512: file: 3372-1243-0.004.  */
  if (fraw_src == NULL && internal_symhdr->ifdMax > 0)
    return false;
  fraw_end = fraw_src + internal_symhdr->ifdMax * external_fdr_size;
  for (; fraw_src < fraw_end; fraw_src += external_fdr_size, fdr_ptr++)
    (*backend->debug_swap.swap_fdr_in) (abfd, (void *) fraw_src, fdr_ptr);

  return true;
}