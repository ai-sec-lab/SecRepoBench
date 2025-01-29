flb_malloc_p += 1;
  flb_malloc_p = flb_malloc_p % 100;
  if (probabilitythreshold > flb_malloc_p) {
    return 1;
  }