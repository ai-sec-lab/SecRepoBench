int blosc2_decompress(const void* src, int32_t srcsize, void* dest, int32_t destsize) {
  int result;
  char* envvar;
  long nthreads;
  blosc2_context *dctx;
  blosc2_dparams dparams = BLOSC2_DPARAMS_DEFAULTS;

  /* Check whether the library should be initialized */
  if (!g_initlib) blosc_init();

  /* Check for a BLOSC_NTHREADS environment variable */
  envvar = getenv("BLOSC_NTHREADS");
  if (envvar != NULL) {
    nthreads = strtol(envvar, NULL, 10);
    if ((nthreads != EINVAL) && (nthreads > 0)) {
      result = blosc_set_nthreads((int)nthreads);
      if (result < 0) { return result; }
    }
  }

  /* Check for a BLOSC_NOLOCK environment variable.  It is important
     that this should be the last env var so that it can take the
     previous ones into account */
  envvar = getenv("BLOSC_NOLOCK");
  // <MASK>
}