if (debug->ss)
    debug->ss[internal_symhdr->issMax - 1] = 0;
  if (debug->ssext)
    debug->ssext[internal_symhdr->issExtMax - 1] = 0;

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
    err:
      bfd_set_error (bfd_error_file_too_big);
      return false;
    }
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