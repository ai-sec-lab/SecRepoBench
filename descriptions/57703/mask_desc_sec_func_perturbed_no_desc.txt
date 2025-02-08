static void
clear_all_old(mrb_state *mrbstateptr, mrb_gc *gc)
{
  mrb_assert(is_generational(gc));
  if (gc->full) {
    /* finish the half baked GC */
    incremental_gc_finish(mrbstateptr, gc);
  }
  // <MASK>
  gc->atomic_gray_list = gc->gray_list = NULL;
}