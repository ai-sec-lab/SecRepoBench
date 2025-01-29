/* Sweep the dead objects, then reset all the live objects
   * (including all the old objects, of course) to white. */
  gc->generational = FALSE;
  prepare_incremental_sweep(mrb, gc);
  incremental_gc_finish(mrb, gc);
  gc->generational = TRUE;
  /* The gray objects have already been painted as white */