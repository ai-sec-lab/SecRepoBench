#ifdef FLB_HAVE_TESTS_OSSFUZZ
/*
 * Return 1 or 0 based on a probability.
 */
int flb_malloc_p;

static inline int flb_fuzz_get_probability(int val) {
  flb_malloc_p += 1;
  flb_malloc_p = flb_malloc_p % 1000;
  if (val > flb_malloc_p) {
    return 1;
  }
  return 0;
}
#endif