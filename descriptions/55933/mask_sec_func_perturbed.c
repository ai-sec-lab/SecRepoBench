bool
_bfd_ecoff_slurp_symbol_table (bfd *abfd)
{
  const struct ecoff_backend_data * const backend = ecoff_backend (abfd);
  const bfd_size_type external_ext_size
    = backend->debug_swap.external_ext_size;
  const bfd_size_type externalsymbolsize
    = backend->debug_swap.external_sym_size;
  void (* const swap_ext_in) (bfd *, void *, EXTR *)
    = backend->debug_swap.swap_ext_in;
  void (* const swap_sym_in) (bfd *, void *, SYMR *)
    = backend->debug_swap.swap_sym_in;
  ecoff_symbol_type *internal;
  ecoff_symbol_type *internal_ptr;
  char *eraw_src;
  char *eraw_end;
  FDR *fdr_ptr;
  FDR *fdr_end;
  size_t amt;

  /* If we've already read in the symbol table, do nothing.  */
  if (ecoff_data (abfd)->canonical_symbols != NULL)
    return true;

  /* Get the symbolic information.  */
  if (! _bfd_ecoff_slurp_symbolic_info (abfd, NULL,
					&ecoff_data (abfd)->debug_info))
    return false;
  if (bfd_get_symcount (abfd) == 0)
    return true;

  if (_bfd_mul_overflow (bfd_get_symcount (abfd),
			 sizeof (ecoff_symbol_type), &amt))
    {
      bfd_set_error (bfd_error_file_too_big);
      return false;
    }
  internal = (ecoff_symbol_type *) bfd_alloc (abfd, amt);
  if (internal == NULL)
    return false;

  internal_ptr = internal;
  eraw_src = (char *) ecoff_data (abfd)->debug_info.external_ext;
  eraw_end = (eraw_src
	      + (ecoff_data (abfd)->debug_info.symbolic_header.iextMax
		 * external_ext_size));
  // <MASK>

  /* PR 17512: file: 3372-3080-0.004.
     A discrepancy between ecoff_data (abfd)->debug_info.symbolic_header.isymMax
     and ecoff_data (abfd)->debug_info.symbolic_header.ifdMax can mean that
     we have fewer symbols than we were expecting.  Allow for this by updating
     the symbol count and warning the user.  */
  if (internal_ptr - internal < (ptrdiff_t) bfd_get_symcount (abfd))
    {
      abfd->symcount = internal_ptr - internal;
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: warning: isymMax (%ld) is greater than ifdMax (%ld)"),
	 abfd, ecoff_data (abfd)->debug_info.symbolic_header.isymMax,
	 ecoff_data (abfd)->debug_info.symbolic_header.ifdMax);
    }

  ecoff_data (abfd)->canonical_symbols = internal;

  return true;
}