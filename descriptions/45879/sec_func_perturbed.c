static inline int flb_fuzz_get_probability(int probabilitythreshold) {
  flb_malloc_p += 1;
  flb_malloc_p = flb_malloc_p % 1000;
  if (probabilitythreshold > flb_malloc_p) {
    return 1;
  }
  return 0;
}