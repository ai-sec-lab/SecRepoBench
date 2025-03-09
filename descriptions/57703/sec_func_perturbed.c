static void
clear_all_old(mrb_state *mrbstateptr, mrb_gc *gc)
{
  mrb_assert(is_generational(gc));
  if (gc->full) {
    /* finish the half baked GC */
    incremental_gc_finish(mrbstateptr, gc);
  }
  /* Sweep the dead objects, then reset all the live objects
   * (including all the old objects, of course) to white. */
  gc->generational = FALSE;
  prepare_incremental_sweep(mrbstateptr, gc);
  incremental_gc_finish(mrbstateptr, gc);
  gc->generational = TRUE;
  /* The gray objects have already been painted as white */
  gc->atomic_gray_list = gc->gray_list = NULL;
}